#include <Arduino.h>
#include <Adafruit_NeoTrellis.h>

Adafruit_seesaw sensor [4];
bool sensorStatus[4];
int baseAddress = 0x36;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("ONLINE");

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
    delay(2000);
}