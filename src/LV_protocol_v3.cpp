#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#define LED_GPIO_PIN 5

#define NUM_OF_NODES 3
#define NUM_OF_NODES_FOR_REC 3  // If there are too many nodes, then the LV protocol reads only the nearest

float state_group[NUM_OF_NODES];
float rec_state_group[NUM_OF_NODES][NUM_OF_NODES];
String bssid_group[NUM_OF_NODES];
int rssi_group[NUM_OF_NODES];

int num_of_rec_mes = 0;
const float alpha = 0.7;  // Algorithm sensitivity factor
const float epsilon = 0.2;  // Epsilon consensus

static wifi_country_t wifi_country = {.cc = "CN", .schan = 1, .nchan = 13};

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer_init(void);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

static int get_node_index();
static int get_state_node();
static void lv_protocol_init();
static char *get_ap_ssid();
static void update_state_group();
static bool is_stabilization();
static void refresh_rec_info();
static void print_status_state_group();
static void print_state_group();
static void print_rec_message();

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

void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    if (num_of_rec_mes >= NUM_OF_NODES_FOR_REC) return;
    if (num_of_rec_mes >= NUM_OF_NODES) return;

    const wifi_promiscuous_pkt_t *ppkt = reinterpret_cast<wifi_promiscuous_pkt_t *>(buff);

    if (ppkt->payload[38] == 49 && ppkt->payload[39] == 49 && ppkt->payload[40] == 49) {  // Group ID check
        String bssid = (String)ppkt->payload[10] + (String)ppkt->payload[11] + (String)ppkt->payload[12] +
                (String)ppkt->payload[13] + (String)ppkt->payload[14] + (String)ppkt->payload[15];

        // int bssid1[6] = {148, 185, 126, 233, 155, 253};
        // if (bssid[0] == bssid1[0] && bssid[1] == bssid1[1] && bssid[2] == bssid1[2]
        //    && bssid[3] == bssid1[3] && bssid[4] == bssid1[4] && bssid[5] == bssid1[5]) return;

        for (int i = 0; i < num_of_rec_mes; i++) {
            if (bssid_group[i] == bssid) return;
        }

        for (int i = 0; i < NUM_OF_NODES; i++) {
            rec_state_group[num_of_rec_mes][i] = (10 * static_cast<float>(ppkt->payload[41 + 2 * i] - '0') +
                    static_cast<float>(ppkt->payload[42 + 2 * i] - '0')) / 10;
        }

        bssid_group[num_of_rec_mes] = bssid;
        rssi_group[num_of_rec_mes] = ppkt->rx_ctrl.rssi;
        num_of_rec_mes++;
    }
}

int get_node_index() {
    return 0;  // Indexing from 0 to NUM_OF_NODES - 1
}

int get_state_node() {
    return 9;
}

#define get_state_node() 9

void lv_protocol_init() {
    for (int i = 0; i < NUM_OF_NODES; i++) {
        for (int j = 0; j < NUM_OF_NODES; j++) {
            rec_state_group[i][j] = 0;
        }
        state_group[i] = 0;
        bssid_group[i] = "";
        rssi_group[i] = -100;
    }
}

char *get_ap_ssid() {
    char *ap_ssid = new char[4 + NUM_OF_NODES * 2]
    {'1', '1', '1'};  // Group ID
    for (int i = 0; i < NUM_OF_NODES; i++) {
        char first_number = static_cast<int>(state_group[i]) + '0';
        char second_number = static_cast<int>(10 * state_group[i]) % 10 + '0';
        ap_ssid[3 + 2 * i] = first_number;
        ap_ssid[4 + 2 * i] = second_number;
    }
    ap_ssid[3 + NUM_OF_NODES * 2] = '\0';

    return ap_ssid;
}

// Update data according to the LV-protocol
void update_state_group() {
    for (int i = 0; i < num_of_rec_mes; i++) {
        for (int j = 0; j < NUM_OF_NODES; j++) {
            if (j == get_node_index()) continue;
            state_group[j] += alpha * (rec_state_group[i][j] - state_group[j]);
        }
    }
    state_group[get_node_index()] = get_state_node();
}

bool is_stabilization() {
    for (int i = 0; i < num_of_rec_mes; i++) {
        for (int j = 0; j < NUM_OF_NODES; j++) {
            if (abs(rec_state_group[i][j] - state_group[j]) > epsilon) {
                return false;
            }
        }
    }
    return true;
}

void refresh_rec_info() {
    num_of_rec_mes = 0;
    for (int i = 0; i < NUM_OF_NODES; i++) {
        bssid_group[i] = "";
        rssi_group[i] = -100;  // Minimum signal strength value
    }
}

void print_status_state_group() {
    Serial.print("Stabilization status: ");
    if (num_of_rec_mes == 0) {
        Serial.println("Not found networks");
    } else if (is_stabilization()) {
        Serial.println("Stabilization");
    } else {
        Serial.println("Non stabilization");
    }
}

void print_state_group() {
    Serial.println("State group:");
    for (int i = 0; i < NUM_OF_NODES; i++) {
        Serial.print("Index node " + String(i) + ": ");
        Serial.println(state_group[i]);
    }
}

void print_rec_message() {
    Serial.println("Recieved message:");
    for (int i = 0; i < num_of_rec_mes; i++) {
        Serial.println("Message from node with BSSID = " + bssid_group[i] + ":");
        for (int j = 0; j < NUM_OF_NODES; j++) {
            Serial.print(rec_state_group[i][j]);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void setup() {
    Serial.begin(9600);
    delay(10);
    wifi_sniffer_init();
    lv_protocol_init();
    pinMode(LED_GPIO_PIN, OUTPUT);
}

// int counter = 0;

void loop() {
    Serial.println("Loop");

    if (digitalRead(LED_GPIO_PIN) == LOW)
        digitalWrite(LED_GPIO_PIN, HIGH);
    else
        digitalWrite(LED_GPIO_PIN, LOW);

    // if (Serial.available() > 0) {
    // state = Serial.parseInt();
    // state_lv = state;
    // } else if (is_sender) {
    //     state_lv = state;
    // } else {
    //     state = round(state_lv);
    // }

    char *ap_ssid = get_ap_ssid();

    WiFi.softAP(ap_ssid, NULL);

    update_state_group();

    Serial.println();
    print_status_state_group();
    Serial.println();
    print_state_group();
    Serial.println();
    print_rec_message();

    refresh_rec_info();
    delay(5000);

    // if (counter >= 600) ESP.restart();
    // else
    //     counter++;
}
