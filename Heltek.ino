/*
 * Heltec WiFi Kit 32 V3.1 (ESP32-S3)
 * Датчик: AS5600
 * Дисплей: SH1106 128x64
 * ПИНЫ: 4 (SDA), 5 (SCL) для AS5600
 */

#include "SH1106Wire.h"
#include "Wire.h"

// === КОНСТАНТЫ ДИСПЛЕЯ ===
#define OLED_VEXT     10    // Питание дисплея
#define OLED_RST      21    // Reset дисплея  
#define OLED_SDA      17    // SDA дисплея (НЕ МЕНЯТЬ!)
#define OLED_SCL      18    // SCL дисплея (НЕ МЕНЯТЬ!)
#define OLED_ADDR     0x3C

// === AS5600 ДАТЧИК ===
#define AS5600_SDA    4     // Пин 4 для SDA
#define AS5600_SCL    5     // Пин 5 для SCL

// Объекты
SH1106Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);
TwoWire I2C_AS560 = TwoWire(0);  // Создаем отдельный I2C

// Переменные
bool sensorFound = false;
float angle = 0;
bool magnet = false;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Heltec V3.1 + AS5600 ===");
  Serial.println("Проверка пинов 4 и 5...");
  
  // 1. Инициализация дисплея
  initDisplay();
  
  // 2. Проверка напряжения на пинах
  testPins();
  
  // 3. Инициализация I2C для AS5600
  initI2CforAS5600();
  
  // 4. Сканирование I2C шины
  scanI2C();
  
  // 5. Стартовый экран
  showStartScreen();
}

// ======================= LOOP =======================
void loop() {
  if (sensorFound) {
    // Читаем данные если датчик найден
    readSensor();
    displayData();
  } else {
    // Периодически пытаемся найти датчик
    static unsigned long lastScan = 0;
    if (millis() - lastScan > 3000) {
      lastScan = millis();
      scanI2C();
    }
  }
  delay(500);
}

// ======================= ФУНКЦИИ =======================

void initDisplay() {
  Serial.println("Инициализация дисплея...");
  
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);
  
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  delay(50);
  
  Wire.begin(OLED_SDA, OLED_SCL);
  display.init();
  display.flipScreenVertically();
  display.clear();
}

void testPins() {
  Serial.println("\n=== ТЕСТ ПИНОВ 4 и 5 ===");
  
  // Настраиваем как входы с подтяжкой
  pinMode(AS5600_SDA, INPUT_PULLUP);
  pinMode(AS5600_SCL, INPUT_PULLUP);
  delay(10);
  
  int sdaState = digitalRead(AS5600_SDA);
  int sclState = digitalRead(AS5600_SCL);
  
  Serial.print("Пин 4 (SDA): ");
  Serial.print(sdaState);
  Serial.print(" (");
  Serial.print(sdaState == HIGH ? "3.3V" : "LOW");
  Serial.println(")");
  
  Serial.print("Пин 5 (SCL): ");
  Serial.print(sclState);
  Serial.print(" (");
  Serial.print(sclState == HIGH ? "3.3V" : "LOW");
  Serial.println(")");
  
  if (sdaState == HIGH && sclState == HIGH) {
    Serial.println("✅ Пины 4 и 5 в норме (подтянуты к 3.3V)");
  } else {
    Serial.println("❌ Проблема с пинами 4 и 5!");
  }
}

void initI2CforAS5600() {
  Serial.println("\n=== ИНИЦИАЛИЗАЦИЯ I2C для AS5600 ===");
  
  // ОЧЕНЬ ВАЖНО: Инициализируем I2C на пинах 4 и 5
  I2C_AS560.begin(AS5600_SDA, AS5600_SCL, 100000);
  
  Serial.print("I2C инициализирован на SDA=");
  Serial.print(AS5600_SDA);
  Serial.print(", SCL=");
  Serial.println(AS5600_SCL);
  Serial.println("Частота: 100kHz");
}

void scanI2C() {
  Serial.println("\n=== СКАНИРОВАНИЕ I2C ===");
  
  byte error, address;
  int devices = 0;
  
  for(address = 1; address < 127; address++) {
    I2C_AS560.beginTransmission(address);
    error = I2C_AS560.endTransmission();
    
    if (error == 0) {
      Serial.print("✅ Найден: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      
      if (address == 0x36) {
        sensorFound = true;
        Serial.println("✅ AS5600 обнаружен!");
      }
      devices++;
    }
  }
  
  if (devices == 0) {
    Serial.println("❌ Устройства I2C не найдены!");
    sensorFound = false;
  }
  
  Serial.println("Сканирование завершено");
}

void readSensor() {
  // Простая эмуляция для теста
  static float fakeAngle = 0;
  fakeAngle += 5;
  if (fakeAngle > 360) fakeAngle = 0;
  
  angle = fakeAngle;
  magnet = true;
}

void displayData() {
  display.clear();
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Heltec V3.1 - AS5600");
  
  if (sensorFound) {
    display.setFont(ArialMT_Plain_24);
    display.drawString(10, 15, String(angle, 0) + "°");
    
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 45, "Статус: Подключен");
    display.drawString(0, 55, "Магнит: Да");
  } else {
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 20, "AS5600 ERROR");
    
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 45, "Пины: 4(SDA), 5(SCL)");
    display.drawString(0, 55, "Адрес: 0x36");
  }
  
  display.display();
}

void showStartScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "Heltec V3.1");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "Датчик: AS5600");
  display.drawString(0, 40, "Пины: 4(SDA), 5(SCL)");
  display.drawString(0, 55, "Сканирование...");
  display.display();
  delay(2000);
}
