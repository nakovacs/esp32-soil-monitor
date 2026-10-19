// ESP32 soil monitor: DHT11 + capacitive soil sensor + SSD1306 OLED dashboard.
// Wiring: OLED SDA=GPIO21, SCL=GPIO22 | DHT11 S=GPIO4 | Soil AOUT=GPIO34
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define DHT_PIN   4
#define DHT_TYPE  DHT11
#define SOIL_PIN  34

#define SCREEN_W  128
#define SCREEN_H  64
#define OLED_ADDR 0x3C   // use the address the I2C scanner found

// CALIBRATION: read the raw value in open air (dry) and in a glass of water (wet),
// then put those numbers here. Higher raw value = drier.
const int SOIL_DRY = 2400;
const int SOIL_WET = 500;

DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, -1);

int readSoilRaw() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(SOIL_PIN);
    delay(5);
  }
  return sum / 10;
}

void drawRow(int y, const char* left, const char* right) {
  display.setTextSize(1);
  display.setCursor(0, y);
  display.print(left);
  if (right[0] != '\0') {
    display.setCursor(SCREEN_W - (int)strlen(right) * 6, y);
    display.print(right);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  dht.begin();
  analogReadResolution(12);  // 0-4095

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED not found. Check wiring and address.");
    while (true) delay(1000);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();
  delay(2000);  // DHT11 needs a moment after power-up
}

void loop() {
  float tempC = dht.readTemperature();
  float hum   = dht.readHumidity();
  int raw     = readSoilRaw();
  int soilPct = constrain(map(raw, SOIL_DRY, SOIL_WET, 0, 100), 0, 100);

  Serial.printf("Temp: %.1f C | Humidity: %.0f %% | Soil raw: %d | Soil: %d %%\n",
                tempC, hum, raw, soilPct);

  // ---- values used only for the display ----
  bool dhtOk = !isnan(tempC) && !isnan(hum);
  float tempF = 0, dewF = 0, absH = 0;
  if (dhtOk) {
    float rh = (hum < 1.0f) ? 1.0f : hum;
    const float a = 17.62f, b = 243.12f;                 // Magnus formula constants
    float g = log(rh / 100.0f) + a * tempC / (b + tempC);
    float dewC = b * g / (a - g);
    float es = 6.112f * exp(a * tempC / (b + tempC));    // saturation vapor pressure, hPa
    absH = 216.74f * es * (rh / 100.0f) / (273.15f + tempC);  // g/m3
    tempF = tempC * 9.0f / 5.0f + 32.0f;
    dewF  = dewC * 9.0f / 5.0f + 32.0f;
  }
  const char* label = (soilPct < 30) ? "DRY" : (soilPct < 60 ? "OK" : "WET");
  long mvSum = 0;
  for (int i = 0; i < 10; i++) {
    mvSum += analogReadMilliVolts(SOIL_PIN);
    delay(2);
  }
  int mv = mvSum / 10;

  char tStr[16], hStr[16], dewStr[16], absStr[16], big[20], mvStr[16], rawStr[16], calStr[24];
  if (dhtOk) {
    snprintf(tStr,   sizeof(tStr),   "T %.1fF", tempF);
    snprintf(hStr,   sizeof(hStr),   "H %.1f%%", hum);
    snprintf(dewStr, sizeof(dewStr), "Dew %.1fF", dewF);
    snprintf(absStr, sizeof(absStr), "AbsH %.1f", absH);
  } else {
    snprintf(tStr,   sizeof(tStr),   "T --");
    snprintf(hStr,   sizeof(hStr),   "H --");
    snprintf(dewStr, sizeof(dewStr), "Dew --");
    snprintf(absStr, sizeof(absStr), "AbsH --");
  }
  snprintf(big,    sizeof(big),    "%d%% %s", soilPct, label);
  snprintf(mvStr,  sizeof(mvStr),  "%d mV", mv);
  snprintf(rawStr, sizeof(rawStr), "raw %d", raw);
  snprintf(calStr, sizeof(calStr), "cal %d..%d", SOIL_WET, SOIL_DRY);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  drawRow(0, tStr, hStr);
  drawRow(9, dewStr, absStr);
  display.drawFastHLine(0, 18, SCREEN_W, SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 21);
  display.print(big);

  display.drawRect(0, 38, SCREEN_W, 9, SSD1306_WHITE);
  int fillW = (int)(124.0f * soilPct / 100.0f);
  if (fillW > 0) display.fillRect(2, 40, fillW, 5, SSD1306_WHITE);

  drawRow(49, mvStr, rawStr);
  drawRow(57, calStr, "");
  display.display();

  delay(1500);  // slower screen update
}
