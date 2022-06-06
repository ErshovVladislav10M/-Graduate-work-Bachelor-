#ifndef LV_PROTOCOL
#define ADD_H

#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

esp_err_t event_handler(void *ctx, system_event_t *event);
void wifi_sniffer_init(void);
void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

int get_num_of_nodes();
int get_num_of_nodes_for_rec();

void set_num_of_nodes(int num);
void set_num_of_nodes_for_rec(int num);

int get_node_index();
float get_state_node();
float get_alpha();  // Algorithm sensitivity factor
float get_epsilon();  // Epsilon consensus

float *get_state_group();
float **get_rec_state_group();
String *get_bssid_group();
int *get_rssi_group();

int get_num_of_rec_mes();

void set_node_index(int index);
void set_state_node(float state);
void set_alpha(float a);
void set_epsilon(float e);

void set_num_of_rec_mes(int num);

void lv_protocol_init();
char *create_message();
void send_message(char *message);
void update_state_group();
bool is_stabilization();
void refresh_rec_info();
void print_status_state_group();
void print_state_group();
void print_rec_message();

#endif
