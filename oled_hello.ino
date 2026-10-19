// Minimal OLED test for ESP32: prints "Hello!" on an SSD1306 128x64 I2C screen
// Wiring: OLED VCC -> 3V3, GND -> GND, SDA -> GPIO21, SCL -> GPIO22
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(21, 22);  // SDA, SCL

  bool ok = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!ok) {
    Serial.println("Nothing at 0x3C, trying 0x3D...");
    ok = display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  }
  if (!ok) {
    Serial.println("OLED not found. Check VCC, GND, SDA, SCL.");
    while (true) delay(1000);
  }
  Serial.println("OLED found!");

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Hello!");
  display.setTextSize(1);
  display.println();
  display.println("OLED is working");
  display.display();
  display.ssd1306_command(0xA5);  // force all pixels on
}

void loop() {
}
