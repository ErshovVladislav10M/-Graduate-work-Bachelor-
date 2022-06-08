#ifndef SRC_LVPROTOCOL_H_
#define SRC_LVPROTOCOL_H_

#include <WiFi.h>

void wifi_sniffer_init(void);

int get_num_of_nodes();
int get_num_of_nodes_for_rec();

void set_num_of_nodes(int num);
void set_num_of_nodes_for_rec(int num);  // If there are too many nodes, then the LV protocol reads only the nearest

int get_this_node_index();
float get_alpha();  // Algorithm sensitivity factor
float get_epsilon();  // Epsilon consensus

float get_state_node(int ind_node);
float *get_rec_message(String bssid);
String *get_bssid_group();
int *get_rssi_group();

int get_num_of_rec_mes();

void set_this_node_index(int index);  // Indexing from 0 to num_of_nodes - 1
void set_state_this_node(float state);
void set_alpha(float a);
void set_epsilon(float e);

void lv_protocol_init();
char *create_message();
void send_message(char *message);
void update_state_group();
bool is_stabilization();
void refresh_rec_info();

void print_status_state_group();
void print_state_group();
void print_rec_message();

#endif  // SRC_LVPROTOCOL_H_
