// I2C scanner for ESP32 (OLED on SDA = GPIO21, SCL = GPIO22)
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(21, 22);  // SDA, SCL
  Serial.println("Scanning I2C bus...");
}

void loop() {
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Device found at 0x");
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("No I2C devices found. Check SDA/SCL wiring and power.");
  }
  Serial.println("---");
  delay(3000);
}
