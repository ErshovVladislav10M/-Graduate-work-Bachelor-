#include <SPI.h>
#include <WiFi.h>

char ssid[] = "111yourNetwork";
char pass[] = "yourPassword";
int status = WL_IDLE_STATUS;

WiFiServer server(80);

void setup() {
    Serial.begin(9600);

    status = WiFi.begin(ssid, NULL);
    if (status != WL_CONNECTED) {
        Serial.println("Couldn't get a wifi connection");
        while (true) {;}
    } else {
        server.begin();
    }
}

void loop() {
    server.write("111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
}
