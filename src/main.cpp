#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoTrellis.h>
#include <Wire.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

const int lineHeight = 12;
const int lineIndent = 4;
const int lineStart = 12;

Adafruit_seesaw sensor [4];
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool sensorStatus[4];
int baseAddress = 0x36;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("ONLINE");
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextWrap(false);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(5,15);
    display.print("AAAAAAAAAA");
    display.display();

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
    display.clearDisplay();
    display.setTextSize(1.5);
    for (int i = 0; i < 4; i++) {
        int line =  lineStart + (lineHeight * i);
        tempC[i] = sensor[i].getTemp();
        capread[i] = sensor[i].touchRead(0);

        display.setCursor(lineIndent, line);
        display.printf("[%i] T: %.1fC M: %i", i, tempC[i], capread[i]);
        Serial.printf("%iT: %fC ", i, tempC[i]);
        Serial.printf("%iC: %i ", i, capread[i]);
    }
    display.display();
    Serial.println();
    delay(2000);
}