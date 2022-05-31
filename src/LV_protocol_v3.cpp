#include <WiFi.h>

#include <LVProtocol.h>

#define LED_GPIO_PIN 5

void setup() {
    Serial.begin(9600);
    delay(10);
    wifi_sniffer_init();
    lv_protocol_init();
    pinMode(LED_GPIO_PIN, OUTPUT);
}

void loop() {
    Serial.println("Loop");

    if (digitalRead(LED_GPIO_PIN) == LOW) {
        digitalWrite(LED_GPIO_PIN, HIGH);
    } else {
        digitalWrite(LED_GPIO_PIN, LOW);
    }

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
}
