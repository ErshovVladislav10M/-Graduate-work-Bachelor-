#include <WiFi.h>

#include <LVProtocol.h>

#define LED_GPIO_PIN 5

void setup() {
    Serial.begin(9600);
    delay(10);

    wifi_sniffer_init();

    set_num_of_nodes(3);
    set_num_of_nodes_for_rec(3);
    set_node_index(0);
    set_state_node(9);
    set_alpha(0.9);
    set_epsilon(0.2);

    lv_protocol_init();

    pinMode(LED_GPIO_PIN, OUTPUT);
}

void loop() {
    Serial.println("------Loop------");

    if (digitalRead(LED_GPIO_PIN) == LOW) {
        digitalWrite(LED_GPIO_PIN, HIGH);
    } else {
        digitalWrite(LED_GPIO_PIN, LOW);
    }

    set_state_node(9);
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
