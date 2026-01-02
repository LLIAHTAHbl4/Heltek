/*
 * Heltec WiFi Kit 32 V3 (ESP32-S3)
 * AS5600 + SHT31 + Battery
 * BLE UART (Nordic)
 */

#include <Wire.h>
#include "SH1106Wire.h"
#include "Adafruit_SHT31.h"
#include <NimBLEDevice.h>

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
// Heltec V3 battery ADC
#define BAT_ADC_PIN 1   // GPIO1

// КАЛИБРОВКА
#define BAT_CAL 1.46
#define BAT_MIN 3.3
#define BAT_MAX 4.2

// ================= BLE UART =================
#define UART_SERVICE "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_RX      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define UART_TX      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

NimBLECharacteristic *txChar;

// ================= GLOBALS =================
float angleDeg, temperature, humidity;
float batteryVoltage;
int batteryPercent;
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

int batPercent(float v) {
  if (v >= 4.2) return 100;
  if (v >= 4.1) return 90;
  if (v >= 4.0) return 80;
  if (v >= 3.9) return 65;
  if (v >= 3.8) return 50;
  if (v >= 3.7) return 35;
  if (v >= 3.6) return 20;
  if (v >= 3.5) return 10;
  return 0;
}

// ================= AS5600 =================
uint16_t readAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() == 2) {
    return (Wire.read() << 8) | Wire.read();
  }
  return 0;
}

// ================= BLE =================
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer*) override {}
  void onDisconnect(NimBLEServer*) override {
    NimBLEDevice::startAdvertising();
  }
};

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

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

  sht31.begin(SHT31_ADDR);

  // BLE
  NimBLEDevice::init("Heltec_V3_BLE");
  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  NimBLEService *svc = server->createService(UART_SERVICE);
  txChar = svc->createCharacteristic(UART_TX, NIMBLE_PROPERTY::NOTIFY);
  svc->createCharacteristic(UART_RX, NIMBLE_PROPERTY::WRITE);

  svc->start();
  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(UART_SERVICE);
  adv->start();
}

// ================= LOOP =================
void loop() {
  if (millis() - lastUpdate < 500) return;
  lastUpdate = millis();

  angleDeg = (readAS5600() * 360.0) / 4096.0;
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  batteryVoltage = readBattery();
  batteryPercent = batPercent(batteryVoltage);

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Angle: " + String(angleDeg,1));
  display.drawString(0, 12, "T: " + String(temperature,1));
  display.drawString(0, 24, "H: " + String(humidity,1));
  display.drawString(0, 36, String(batteryVoltage,2) + "V " + String(batteryPercent) + "%");
  display.display();

  String msg =
    "Angle=" + String(angleDeg,1) +
    " T=" + String(temperature,1) +
    " H=" + String(humidity,1) +
    " Bat=" + String(batteryVoltage,2) +
    "V " + String(batteryPercent) + "%";

  Serial.println(msg);
  txChar->setValue(msg);
  txChar->notify();
}
