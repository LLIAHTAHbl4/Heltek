/*
 * Heltec WiFi Kit 32 V3 (ESP32-S3)
 * AS5600 + SHT31 + Battery
 * OUTPUT: OLED ONLY
 */

#include <Wire.h>
#include "SH1106Wire.h"
#include "Adafruit_SHT31.h"

// ================= I2C =================
#define SDA_PIN 17
#define SCL_PIN 18

// ================= OLED =================
#define OLED_ADDR 0x3C
#define OLED_VEXT 10
#define OLED_RST  21

SH1106Wire display(OLED_ADDR, SDA_PIN, SCL_PIN);

// ================= SENSORS =================
#define AS5600_ADDR 0x36
#define SHT31_ADDR  0x44
Adafruit_SHT31 sht31;

// ================= BATTERY =================
// Heltec WiFi Kit 32 V3 battery ADC
#define BAT_ADC_PIN 1   // GPIO1 (внутренний делитель)

// КАЛИБРОВКА (под твою плату)
#define BAT_CAL 1.46
#define BAT_MIN 3.3
#define BAT_MAX 4.2

// ================= GLOBALS =================
float angleDeg = 0.0;
float temperature = 0.0;
float humidity = 0.0;
float batteryVoltage = 0.0;
int batteryPercent = 0;

unsigned long lastUpdate = 0;

// ================= BATTERY =================
float readBattery() {
  uint32_t sum = 0;
  for (int i = 0; i < 32; i++) {
    sum += analogRead(BAT_ADC_PIN);
    delay(2);
  }

  float adc = sum / 32.0;
  float v = (adc / 4095.0) * 3.3;
  return v * BAT_CAL;
}

int batteryToPercent(float v) {
  if (v >= 4.20) return 100;
  if (v >= 4.10) return 90;
  if (v >= 4.00) return 80;
  if (v >= 3.90) return 65;
  if (v >= 3.80) return 50;
  if (v >= 3.70) return 35;
  if (v >= 3.60) return 20;
  if (v >= 3.50) return 10;
  return 0;
}

// ================= AS5600 =================
uint16_t readAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C);               // RAW ANGLE
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, 2);

  if (Wire.available() == 2) {
    return (Wire.read() << 8) | Wire.read();
  }
  return 0;
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(300);

  // ADC
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // OLED power
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(50);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // OLED init
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();

  // SHT31
  sht31.begin(SHT31_ADDR);

  // Start screen
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Heltec WiFi Kit 32 V3");
  display.drawString(0, 12, "AS5600 + SHT31");
  display.drawString(0, 24, "Battery monitor");
  display.display();
  delay(1500);
}

// ================= LOOP =================
void loop() {
  if (millis() - lastUpdate < 500) return;
  lastUpdate = millis();

  // AS5600
  uint16_t raw = readAS5600();
  angleDeg = (raw * 360.0) / 4096.0;

  // SHT31
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();

  // Battery
  batteryVoltage = readBattery();
  batteryPercent = batteryToPercent(batteryVoltage);

  // OLED
  display.clear();

  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, String(angleDeg, 1) + "°");

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 22, "T: " + String(temperature, 1) + " C");
  display.drawString(0, 34, "H: " + String(humidity, 1) + " %");
  display.drawString(
    0,
    46,
    String(batteryVoltage, 2) + "V  " + String(batteryPercent) + "%"
  );

  display.display();

  // Serial debug (по USB, если нужно)
  Serial.print("Angle=");
  Serial.print(angleDeg, 1);
  Serial.print(" T=");
  Serial.print(temperature, 1);
  Serial.print(" H=");
  Serial.print(humidity, 1);
  Serial.print(" Bat=");
  Serial.print(batteryVoltage, 2);
  Serial.print("V ");
  Serial.print(batteryPercent);
  Serial.println("%");
}
