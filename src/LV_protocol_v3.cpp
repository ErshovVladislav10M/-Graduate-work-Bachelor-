#include <WiFi.h>

#include <LVProtocol.h>

#define LED_GPIO_PIN 5

LVProtocolSettings settings;
LVProtocolState state;
LVProtocolManagerMessage manager;

void setup() {
    Serial.begin(9600);
    delay(10);

    wifi_sniffer_init();

    int num_of_nodes = 3;
    int num_of_nodes_for_rec = 3;
    double alpha = 0.7;
    double epsilon = 0.2;
    LVProtocolSettings settings(num_of_nodes, num_of_nodes_for_rec, alpha, epsilon);

    int node_index = 0;
    double state_node = 9;
    LVProtocolState state(settings, node_index, state_node);

    LVProtocolManagerMessage manager(settings);

    pinMode(LED_GPIO_PIN, OUTPUT);
}

void loop() {
    Serial.println("Loop");

    if (digitalRead(LED_GPIO_PIN) == LOW) {
        digitalWrite(LED_GPIO_PIN, HIGH);
    } else {
        digitalWrite(LED_GPIO_PIN, LOW);
    }

    state.set_state_node(9);
    char *message = create_message(settings, state);
    send_message(message);

    state.update_state_group(settings, manager);

    Serial.println();
    state.print_state_group(settings);
    Serial.println();
    state.print_state_group(settings);
    Serial.println();
    manager.print_rec_message(settings);

    manager.refresh_rec_info(settings);
    delay(5000);
}
