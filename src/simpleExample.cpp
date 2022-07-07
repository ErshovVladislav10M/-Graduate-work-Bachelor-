#include <WiFi.h>

#include <LVProtocol.h>

void setup() {
    Serial.begin(9600);
    delay(10);

    wifi_sniffer_init();

    set_num_of_nodes(3);
    set_num_of_nodes_for_rec(3);
    set_this_node_index(2);
    set_state_this_node(1);
    set_alpha(0.9);
    set_epsilon(0.2);

    lv_protocol_init();
}

void loop() {
    Serial.println("------Loop------");

    set_state_this_node(1);  // Set your's state node
    char *message = create_message();
    send_message(message);

    update_state_group();

    Serial.println();
    print_status_state_group();
    Serial.println();
    print_state_group();
    Serial.println();
    print_rec_message();
    Serial.println();

    refresh_rec_info();
    delay(5000);
}
