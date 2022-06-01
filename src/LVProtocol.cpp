#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#include <LVProtocol.h>

class LVProtocolSettings {
    int num_of_nodes;
    int num_of_nodes_for_rec;  // If there are too many nodes, then the LV protocol reads only the nearest

    double alpha;
    double epsilon;

 public:
    explicit LVProtocolSettings(int nodes, int nodes_for_rec, double a, double e) {
        num_of_nodes = nodes;
        num_of_nodes_for_rec = nodes_for_rec;
        alpha = a;
        epsilon = e;
    }

    int get_num_of_nodes() {
        return num_of_nodes;
    }

    int get_num_of_nodes_for_rec() {
        return num_of_nodes_for_rec;
    }

    void set_num_of_nodes(int num) {
        num_of_nodes = num;
    }

    void set_num_of_nodes_for_rec(int num) {
        num_of_nodes_for_rec = num;
    }

    double get_alpha() {
        return alpha;
    }

    double get_epsilon() {
        return epsilon;
    }

    void set_alpha(double a) {
        alpha = a;
    }

    void set_epsilon(double e) {
        epsilon = e;
    }
};

class LVProtocolState {
    int node_index;  // Indexing from 0 to num_of_nodes - 1
    double state_node;

    double *state_group;

 public:
    explicit LVProtocolState(LVProtocolSettings settings, int index, double state) {
        node_index = index;
        state_node = state;
        state_group = new double[settings.get_num_of_nodes()];
        for (int i = 0; i < settings.get_num_of_nodes(); i++) {
            state_group[i] = 0;
        }
    }

    int get_node_index() {
        return node_index;
    }

    double get_state_node() {
        return state_node;
    }

    double *get_state_group() {
        return state_group;
    }

    void set_node_index(int index) {
        node_index = index;
    }

    void set_state_node(double state) {
        state_node = state;
    }

    // Update data according to the LV-protocol
    void update_state_group(LVProtocolSettings settings, LVProtocolManagerMessage manager) {
        for (int i = 0; i < manager.get_num_of_rec_mes(); i++) {
            for (int j = 0; j < settings.get_num_of_nodes(); j++) {
                if (j == node_index) continue;
                state_group[j] += settings.get_alpha() * (manager.get_rec_state_group()[i][j] - state_group[j]);
            }
        }
        state_group[node_index] = state_node;
    }

    bool is_stabilization(LVProtocolSettings settings, LVProtocolManagerMessage manager) {
        for (int i = 0; i < manager.get_num_of_rec_mes(); i++) {
            for (int j = 0; j < settings.get_num_of_nodes(); j++) {
                if (abs(manager.get_rec_state_group()[i][j] - state_group[j]) > settings.get_epsilon()) {
                    return false;
                }
            }
        }
        return true;
    }

    void print_status_state_group(LVProtocolSettings settings, LVProtocolManagerMessage manager) {
        Serial.print("Network status: ");
        if (manager.get_num_of_rec_mes() == 0) {
            Serial.println("Not found networks");
        } else if (is_stabilization(settings, manager)) {
            Serial.println("Stabilization");
        } else {
            Serial.println("Non stabilization");
        }
    }

    void print_state_group(LVProtocolSettings settings) {
        Serial.println("State group:");
        for (int i = 0; i < settings.get_num_of_nodes(); i++) {
            Serial.print("Index node " + String(i) + ": ");
            Serial.println(state_group[i]);
        }
    }
};

class LVProtocolManagerMessage {
    double **rec_state_group;
    String *bssid_group;
    int *rssi_group;

    int num_of_rec_mes = 0;

 public:
    explicit LVProtocolManagerMessage(LVProtocolSettings settings) {
        rec_state_group = new double *[settings.get_num_of_nodes()];
        for (int i = 0; i < settings.get_num_of_nodes(); i++) {
            rec_state_group[i] = new double[settings.get_num_of_nodes()];
        }
        bssid_group = new String[settings.get_num_of_nodes()];
        rssi_group = new int[settings.get_num_of_nodes()];

        for (int i = 0; i < settings.get_num_of_nodes(); i++) {
            for (int j = 0; j < settings.get_num_of_nodes(); j++) {
                rec_state_group[i][j] = 0;
            }
            bssid_group[i] = "";
            rssi_group[i] = -100;
        }
    }

    double **get_rec_state_group() {
        return rec_state_group;
    }

    String *get_bssid_group() {
        return bssid_group;
    }

    int *get_rssi_group() {
        return rssi_group;
    }

    int get_num_of_rec_mes() {
        return num_of_rec_mes;
    }

    void set_num_of_rec_mes(int num) {
        num_of_rec_mes = num;
    }

    void refresh_rec_info(LVProtocolSettings settings) {
        num_of_rec_mes = 0;
        for (int i = 0; i < settings.get_num_of_nodes(); i++) {
            bssid_group[i] = "";
            rssi_group[i] = -100;  // Minimum signal strength value
        }
    }

    void print_rec_message(LVProtocolSettings settings) {
        Serial.println("Recieved message:");
        for (int i = 0; i < num_of_rec_mes; i++) {
            Serial.println("Message from node with BSSID = " + bssid_group[i] + " :");
            for (int j = 0; j < settings.get_num_of_nodes(); j++) {
                Serial.print(rec_state_group[i][j]);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
};

wifi_country_t wifi_country = {.cc = "CN", .schan = 1, .nchan = 13};

esp_err_t event_handler(void *ctx, system_event_t *event) {
    return ESP_OK;
}

void wifi_sniffer_init(void) {
    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type,
                                LVProtocolSettings settings, LVProtocolState state, LVProtocolManagerMessage manager) {
    if (type != WIFI_PKT_MGMT) return;
    if (manager.get_num_of_rec_mes() >= settings.get_num_of_nodes_for_rec()) return;
    if (manager.get_num_of_rec_mes() >= settings.get_num_of_nodes()) return;

    const wifi_promiscuous_pkt_t *ppkt = reinterpret_cast<wifi_promiscuous_pkt_t *>(buff);

    if (ppkt->payload[38] == 49 && ppkt->payload[39] == 49 && ppkt->payload[40] == 49) {  // Group ID check
        String bssid = String(ppkt->payload[10], HEX) + ":" + String(ppkt->payload[11], HEX) + ":" +
                    String(ppkt->payload[12], HEX) + ":" + String(ppkt->payload[13], HEX) + ":" +
                    String(ppkt->payload[14], HEX) + ":" + String(ppkt->payload[15], HEX);

        for (int i = 0; i < manager.get_num_of_rec_mes(); i++) {
            if (manager.get_bssid_group()[i] == bssid) return;
        }

        for (int i = 0; i < settings.get_num_of_nodes(); i++) {
            manager.get_rec_state_group()[manager.get_num_of_rec_mes()][i] = (10 *
                    static_cast<double>(ppkt->payload[41 + 2 * i] - '0') +
                    static_cast<double>(ppkt->payload[42 + 2 * i] - '0')) / 10;
        }

        manager.get_bssid_group()[manager.get_num_of_rec_mes()] = bssid;
        manager.get_rssi_group()[manager.get_num_of_rec_mes()] = ppkt->rx_ctrl.rssi;
        manager.set_num_of_rec_mes(manager.get_num_of_rec_mes() + 1);
    }
}

char *create_message(LVProtocolSettings settings, LVProtocolState state) {
    char *ap_ssid = new char[4 + settings.get_num_of_nodes() * 2]
    {'1', '1', '1'};  // Group ID
    for (int i = 0; i < settings.get_num_of_nodes(); i++) {
        char first_number = static_cast<int>(state.get_state_group()[i]) + '0';
        char second_number = static_cast<int>(10 * state.get_state_group()[i]) % 10 + '0';
        ap_ssid[3 + 2 * i] = first_number;
        ap_ssid[4 + 2 * i] = second_number;
    }
    ap_ssid[3 + settings.get_num_of_nodes() * 2] = '\0';

    return ap_ssid;
}

void send_message(char *message) {
    WiFi.softAP(message, NULL);
}
