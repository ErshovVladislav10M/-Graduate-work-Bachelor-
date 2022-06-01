#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <nvs_flash.h>
#include <driver/gpio.h>

class LVProtocolSettings {
    int num_of_nodes;
    int num_of_nodes_for_rec;  // If there are too many nodes, then the LV protocol reads only the nearest
    double alpha;
    double epsilon;

 public:
    explicit LVProtocolSettings(int nodes, int nodes_for_rec, double a, double e);
    int get_num_of_nodes();
    int get_num_of_nodes_for_rec();
    void set_num_of_nodes(int num);
    void set_num_of_nodes_for_rec(int num);
    double get_alpha();
    double get_epsilon();
    void set_alpha(double a);
    void set_epsilon(double e);
};

class LVProtocolState {
    int node_index;  // Indexing from 0 to num_of_nodes - 1
    double state_node;
    double *state_group;

 public:
    explicit LVProtocolState(LVProtocolSettings settings, int index, double state);
    int get_node_index();
    double get_state_node();
    double *get_state_group();
    void set_node_index(int index);
    void set_state_node(double state);
    // Update data according to the LV-protocol
    void update_state_group(LVProtocolSettings settings, LVProtocolManagerMessage manager);
    bool is_stabilization(LVProtocolSettings settings, LVProtocolManagerMessage manager);
    void print_status_state_group(LVProtocolSettings settings, LVProtocolManagerMessage manager);
    void print_state_group(LVProtocolSettings settings);
};

class LVProtocolManagerMessage {
    double **rec_state_group;
    String *bssid_group;
    int *rssi_group;
    int num_of_rec_mes;

 public:
    explicit LVProtocolManagerMessage(LVProtocolSettings settings);
    double **get_rec_state_group();
    String *get_bssid_group();
    int *get_rssi_group();
    int get_num_of_rec_mes();
    void set_num_of_rec_mes(int num);
    void refresh_rec_info(LVProtocolSettings settings);
    void print_rec_message(LVProtocolSettings settings);
};

esp_err_t event_handler(void *ctx, system_event_t *event);
void wifi_sniffer_init(void);
void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

char *create_message(LVProtocolSettings settings, LVProtocolState state);
void send_message(char *message);
