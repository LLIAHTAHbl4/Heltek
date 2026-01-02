/*
 * Heltec WiFi Kit 32 V3
 * AS5600 + SHT31 + Battery + Bluetooth Classic (SPP)
 * ВЕСЬ КОД ЦЕЛИКОМ
 */

#include <Wire.h>
#include "SH1106Wire.h"
#include "Adafruit_SHT31.h"
#include "BluetoothSerial.h"

// ================== BLUETOOTH ==================
BluetoothSerial SerialBT;

// ================== I2C ==================
#define SDA_PIN 17
#define SCL_PIN 18

// ================== OLED ==================
#define OLED_ADDR 0x3C
#define OLED_VEXT 10
#define OLED_RST  21

SH1106Wire display(OLED_ADDR, SDA_PIN, SCL_PIN);

// ================== SENSORS ==================
#define AS5600_ADDR 0x36
#define SHT31_ADDR  0x44

Adafruit_SHT31 sht31;

// ================== BATTERY ==================
// Heltec V3 встроенный делитель
#define BAT_ADC ADC1_CHANNEL_0

// ⚠️ КАЛИБРОВКА (ПОД ТВОЮ ПЛАТУ)
#define BAT_CALIBRATION 1.46   // подобрано под 4.17V
#define BAT_MIN 3.3
#define BAT_MAX 4.2

// ================== GLOBALS ==================
float angleDeg = 0;
float temperature = 0;
float humidity = 0;
float batteryVoltage = 0;
int batteryPercent = 0;

unsigned long lastUpdate = 0;

// ================== UTILS ==================
float readBatteryFiltered() {
  uint32_t sum = 0;

  // Усреднение 32 измерения
  for (int i = 0; i < 32; i++) {
    sum += analogRead(BAT_ADC);
    delay(2);
  }

  float adc = sum / 32.0;

  // 12 бит, 3.3V
  float v = (adc / 4095.0) * 3.3;

  // КАЛИБРОВКА
  return v * BAT_CALIBRATION;
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

// ================== AS5600 ==================
uint16_t readAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, 2);

  if (Wire.available() == 2) {
    uint16_t hi = Wire.read();
    uint16_t lo = Wire.read();
    return (hi << 8) | lo;
  }
  return 0;
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(500);

  SerialBT.begin("Heltec_V3");
  Serial.println("Bluetooth SPP started");
  SerialBT.println("Heltec V3 Bluetooth OK");

  // ADC
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // OLED
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(50);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);

  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();

  // SHT31
  sht31.begin(SHT31_ADDR);

  SerialBT.println("Setup complete");
}

// ================== LOOP ==================
void loop() {
  if (millis() - lastUpdate < 500) return;
  lastUpdate = millis();

  // ===== AS5600 =====
  uint16_t raw = readAS5600();
  angleDeg = (raw * 360.0) / 4096.0;

  // ===== SHT31 =====
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();

  // ===== BATTERY =====
  batteryVoltage = readBatteryFiltered();
  batteryPercent = batteryToPercent(batteryVoltage);

  // ===== OLED =====
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, String(angleDeg, 1) + "°");

  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 22, "T: " + String(temperature, 1) + "C");
  display.drawString(0, 34, "H: " + String(humidity, 1) + "%");
  display.drawString(0, 46,
    String(batteryVoltage, 2) + "V " +
    String(batteryPercent) + "%");

  display.display();

  // ===== SERIAL + BT =====
  String msg =
    "Angle=" + String(angleDeg, 1) +
    " Temp=" + String(temperature, 1) +
    " Hum=" + String(humidity, 1) +
    " Bat=" + String(batteryVoltage, 2) +
    "V " + String(batteryPercent) + "%";

  Serial.println(msg);
  SerialBT.println(msg);
}
