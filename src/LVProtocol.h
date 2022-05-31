#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

#define NUM_OF_NODES 3
#define NUM_OF_NODES_FOR_REC 3  // If there are too many nodes, then the LV protocol reads only the nearest

esp_err_t event_handler(void *ctx, system_event_t *event);
void wifi_sniffer_init(void);
void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

int get_node_index();
int get_state_node();
float get_alpha();  // Algorithm sensitivity factor
float get_epsilon();  // Epsilon consensus

void lv_protocol_init();
char *get_ap_ssid();
void update_state_group();
bool is_stabilization();
void refresh_rec_info();
void print_status_state_group();
void print_state_group();
void print_rec_message();
