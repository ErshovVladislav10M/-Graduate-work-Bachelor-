#include <WiFi.h>

#include <LVProtocol.h>

#define LED_GPIO_PIN 5

LVProtocolSettings settings(3, 3, 0.7, 0.2);
LVProtocolState state(settings, 0, 9);
LVProtocolManagerMessage manager(settings);

void setup() {
    Serial.begin(9600);
    delay(10);
    wifi_sniffer_init();
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
