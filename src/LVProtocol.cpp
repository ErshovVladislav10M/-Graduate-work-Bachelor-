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
