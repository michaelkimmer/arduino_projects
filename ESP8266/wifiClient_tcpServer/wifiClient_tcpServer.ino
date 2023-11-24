/*
WIFI client with TCP server -- adjusted version of:
https://techtutorialsx.com/2018/06/02/esp8266-arduino-socket-server/
*/

#include "ESP8266WiFi.h"

// Set SSID and PASSWORD !!!
const char* ssid = "ssid";
const char* password = "password";

WiFiServer wifiServer(80);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off

    Serial.begin(115200);
    delay(1000);

    // Connect to Wifi
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting..");
    }

    Serial.print("Connected to WiFi. IP:");
    Serial.println(WiFi.localIP());

    // Start server
    wifiServer.begin();
}

void loop() {
    WiFiClient client = wifiServer.available();

    if (client) {
        while (client.connected()) {
            while (client.available() > 0) {
                char c = client.read();

                // MY -- send the message back + LED (if '0'/'1')
                Serial.write(c);
                client.print(c);
                if (c == '0') digitalWrite(LED_BUILTIN, HIGH);
                if (c == '1') digitalWrite(LED_BUILTIN, LOW);
            }

            delay(10);
        }

        client.stop();
        Serial.println("Client disconnected");
    }
}
