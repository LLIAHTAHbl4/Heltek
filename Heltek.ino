/*
 * Heltec V3 — Правильное измерение батареи
 * BLE + OLED + AS5600 + SHT31 + точная батарея
 */

#include <Wire.h>
#include "SH1106Wire.h"
#include "Adafruit_SHT31.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ===== ПИНЫ =====
#define SDA_PIN   17
#define SCL_PIN   18
#define OLED_VEXT 10
#define OLED_RST  21
#define BAT_ADC   1

// ===== I2C / ADDRESSES =====
#define DISPLAY_ADDR 0x3C
#define AS5600_ADDR  0x36
#define SHT31_ADDR   0x44

// ===== BATTERY CALIBRATION =====
#define ADC_SAMPLES 20

// Делитель на плате ≈ 2.0, но есть доп. калибровка
#define BAT_DIVIDER 2.0
#define BAT_CALIB   1.46   // калибровочный множитель

// ===== BLE UUID для NUS =====
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_TX_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);
Adafruit_SHT31 sht31;
BLECharacteristic *bleTx;

bool as5600Connected = false;
float angleDeg = 0.0;
int magnetPercent = 0;

float temperature = 0;
float humidity = 0;

float batteryVoltage = 0;
int batteryPercent = 0;

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);
  analogSetAttenuation(ADC_6db);

  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);
  display.init();
  display.flipScreenVertically();
  display.clear();

  sht31.begin(SHT31_ADDR);

  BLEDevice::init("Heltec_V3_BLE");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(SERVICE_UUID);
  bleTx = service->createCharacteristic(CHAR_TX_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  bleTx->addDescriptor(new BLE2902());
  service->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
}

void loop() {
  readBattery();
  updateDisplay();
  sendBLE();
  delay(1000);
}

void readBattery() {
  uint32_t sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(BAT_ADC);
    delay(3);
  }

  float adc = sum / (float)ADC_SAMPLES;
  float Vadc = adc * (3.3 / 4095.0);

  // Калиброванное напряжение
  batteryVoltage = Vadc * BAT_DIVIDER * BAT_CALIB;

  // Расчет %
  if (batteryVoltage >= 4.20) batteryPercent = 100;
  else if (batteryVoltage >= 4.00) batteryPercent = map(batteryVoltage * 100, 400, 420, 80, 100);
  else if (batteryVoltage >= 3.85) batteryPercent = map(batteryVoltage * 100, 385, 400, 60, 80);
  else if (batteryVoltage >= 3.75) batteryPercent = map(batteryVoltage * 100, 375, 385, 40, 60);
  else if (batteryVoltage >= 3.60) batteryPercent = map(batteryVoltage * 100, 360, 375, 20, 40);
  else if (batteryVoltage >= 3.30) batteryPercent = map(batteryVoltage * 100, 330, 360, 5, 20);
  else batteryPercent = 0;
}

void updateDisplay() {
  display.clear();
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, "BAT: " + String(batteryVoltage, 2) + "V");
  display.drawString(0, 12, "PCT: " + String(batteryPercent) + "%");

  display.display();
}

void sendBLE() {
  String msg = "Vbat=" + String(batteryVoltage, 3) + "V " + String(batteryPercent) + "%";
  bleTx->setValue(msg.c_str());
  bleTx->notify();
}
