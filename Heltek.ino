#include <Arduino.h>                      // Основные функции Arduino
#include <Wire.h>                         // I2C шина
#include "SSD1306Wire.h"                  // OLED дисплей Heltec
#include <Adafruit_SHT31.h>               // Датчик температуры и влажности
#include <AS5600.h>                       // Магнитный энкодер
#include <NimBLEDevice.h>                 // BLE для ESP32-S3

#define VBAT_PIN 1                        // GPIO1 — измерение батареи на Heltec V3
#define ADC_MAX 4095.0                    // Максимум 12-битного АЦП
#define ADC_REF 3.3                       // Опорное напряжение АЦП
#define VBAT_DIVIDER 2.0                  // Делитель напряжения на плате ~1:2

SSD1306Wire display(0x3C, SDA, SCL);      // Экземпляр OLED дисплея
Adafruit_SHT31 sht31 = Adafruit_SHT31();  // Экземпляр SHT31
AS5600 as5600;                            // Экземпляр AS5600

BLECharacteristic *bleTx;                 // BLE характеристика передачи
bool bleConnected = false;                // Флаг подключения BLE клиента

// ===== CALLBACK BLE СЕРВЕРА =====
class BleCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) {             // При подключении клиента
    bleConnected = true;                   // Запоминаем соединение
  }
  void onDisconnect(BLEServer*) {          // При отключении клиента
    bleConnected = false;                  // Сбрасываем флаг
  }
};

// ===== ЧТЕНИЕ НАПРЯЖЕНИЯ АКБ =====
float readBattery() {
  uint32_t sum = 0;                        // Сумма измерений
  for (int i = 0; i < 50; i++) {            // 50 измерений для усреднения
    sum += analogRead(VBAT_PIN);            // Читаем АЦП
    delay(2);                               // Небольшая задержка
  }
  float adc = sum / 50.0;                  // Среднее значение
  return (adc / ADC_MAX) * ADC_REF * VBAT_DIVIDER; // Пересчёт в вольты
}

// ===== ПРОЦЕНТ ЗАРЯДА АКБ =====
int batteryPercent(float v) {
  if (v >= 4.20) return 100;                // Полный заряд
  if (v <= 3.20) return 0;                  // Пусто
  return (int)((v - 3.20) * 100.0 / 1.0);   // Линейная аппроксимация
}

// ===== ОТПРАВКА В BLE =====
void blePrint(String msg) {
  if (bleConnected) {                      // Если клиент подключён
    bleTx->setValue(msg.c_str());           // Передаём строку
    bleTx->notify();                       // Уведомляем клиента
  }
}

// ===== SETUP =====
void setup() {
  analogReadResolution(12);                // Устанавливаем 12 бит АЦП
  analogSetAttenuation(ADC_11db);           // Диапазон до ~3.6V
  pinMode(VBAT_PIN, INPUT);                // Пин батареи как вход

  Wire.begin();                            // Запуск I2C
  display.init();                         // Инициализация дисплея
  display.flipScreenVertically();         // Правильная ориентация
  display.clear();                        // Очистка экрана

  sht31.begin(0x44);                      // Запуск SHT31
  as5600.begin();                         // Запуск AS5600

  BLEDevice::init("Heltec_V3_BLE");        // Имя BLE устройства
  BLEServer *server = BLEDevice::createServer(); // Создаём сервер
  server->setCallbacks(new BleCallbacks());      // Назначаем callback

  BLEService *service = server->createService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // UART сервис

  bleTx = service->createCharacteristic(
    "6E400003-B5A3-F393-E0A9-E50E24DCCA9E",
    NIMBLE_PROPERTY::NOTIFY
  );

  service->start();                       // Запуск сервиса
  BLEDevice::getAdvertising()->start();   // Запуск рекламы BLE
}

// ===== LOOP =====
void loop() {
  float vbat = readBattery();              // Читаем напряжение АКБ
  int percent = batteryPercent(vbat);      // Считаем процент

  display.clear();                         // Очистка экрана
  display.setFont(ArialMT_Plain_16);       // Шрифт
  display.drawString(0, 0, "VBAT: " + String(vbat, 2) + "V"); // Напряжение
  display.drawString(0, 20, "BAT: " + String(percent) + "%"); // Процент
  display.display();                       // Обновляем экран

  blePrint("VBAT=" + String(vbat, 2) + "V " + String(percent) + "%"); // BLE лог

  delay(1000);                             // Пауза 1 секунда
}
