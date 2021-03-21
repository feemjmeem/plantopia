#include <Adafruit_NeoTrellis.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include "config.h"

const char *ssid = WIFI_SSID;
const char *wifi_password = WIFI_PASSWORD;
const char *hostname = HOSTNAME;

const uint16_t ota_port = OTA_PORT;
const char *ota_password = OTA_PASSWORD;

const char *es_host = ES_HOST;
const uint16_t es_port = ES_PORT;
const char *es_index = ES_INDEX;

Adafruit_seesaw sensor [SENSOR_COUNT];
bool sensorStatus[SENSOR_COUNT];
int baseAddress = SENSOR_BASEADDRESS;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

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

    for (int i = 0; i < SENSOR_COUNT; i++) {
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
    timeClient.begin();
}

void loop() {
    float tempC[SENSOR_COUNT];
    uint16_t capread[SENSOR_COUNT];
    WiFiClient client;

    if (!client.connect(es_host, es_port)) {
        Serial.println("Can't connect to elasticsearch");
        return;
    }

    for (int i = 0; i < SENSOR_COUNT; i++) {
        timeClient.update();
        unsigned long time = timeClient.getEpochTime();
        tempC[i] = sensor[i].getTemp();
        capread[i] = sensor[i].touchRead(0);

        Serial.printf("%iT: %fC ", i, tempC[i]);
        Serial.printf("%iC: %i ", i, capread[i]);
        String data = "{\"capacitance\": " + String(capread[i]) + "," +
                      "\"sensor\": " + String(i) + "," +
                      "\"temperature\": " + String(tempC[i]) + "," +
                      "\"timestamp\": " + String(time) + "}";
        String post = "POST /" + String(es_index) + "/_doc HTTP/1.1\r\n" + 
                      "Content-Length: " + data.length() + "\r\n" +
                      "Content-Type: application/json" + "\r\n" +
                      "\r\n" +
                      data;
        Serial.println();
        Serial.println(post);
        client.print(post);
        delay(50);
        while(client.available()) {
            String line = client.readStringUntil('\r');
            Serial.print(line);
        }
    }
    ArduinoOTA.handle();
    delay(5000);
}