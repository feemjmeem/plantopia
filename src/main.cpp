#include <Adafruit_NeoTrellis.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include "config.h"

const char *ssid = WIFI_SSID;
const char *wifi_password = WIFI_PASSWORD;
const char *hostname = HOSTNAME;

const uint16_t ota_port = OTA_PORT;
const char *ota_password = OTA_PASSWORD;

Adafruit_seesaw sensor [4];
bool sensorStatus[4];
int baseAddress = 0x36;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.printf("\nConnecting...");
    WiFi.hostname(hostname);
    WiFi.begin(ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.printf("Connected!\n");

    ArduinoOTA.setPort(ota_port);
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPassword(ota_password);

    ArduinoOTA.onStart([]() {
        Serial.println("Start updating...");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();

    for (int i = 0; i < 4; i++) {
        int a = baseAddress + i;
        if (!sensorStatus[i]) {
            if (!sensor[i].begin(a)) {
                Serial.printf("NO SENSOR %i\n", i);
                while(1);
            } else {
                Serial.printf("sensor %x started v%x\n", a, sensor[i].getVersion());
                sensorStatus[i] = true;
            }
        }
    }
}

void loop() {
    float tempC[4];
    uint16_t capread[4];
    for (int i = 0; i < 4; i++) {
        tempC[i] = sensor[i].getTemp();
        capread[i] = sensor[i].touchRead(0);

        Serial.printf("%iT: %fC ", i, tempC[i]);
        Serial.printf("%iC: %i ", i, capread[i]);
    }
    Serial.println();
    ArduinoOTA.handle();
    delay(1000);
}