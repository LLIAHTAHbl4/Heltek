/*
 * Быстрый тест GPIO7,6 и GPIO33,34
 */

#include "Wire.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== БЫСТРЫЙ ТЕСТ I2C ПИНОВ ===");
  
  // Тест 1: GPIO7 и GPIO6
  Serial.println("\n1. ТЕСТ: GPIO7,6 (физические пины 12,13)");
  testPair(7, 6);
  
  delay(1000);
  
  // Тест 2: GPIO33 и GPIO34
  Serial.println("\n2. ТЕСТ: GPIO33,34 (Header J3 пины 7,8)");
  testPair(33, 34);
  
  Serial.println("\n=== ТЕСТ ЗАВЕРШЕН ===");
  Serial.println("Подключи SSD1306 к рабочим пинам");
}

void loop() {
  delay(1000);
}

void testPair(int sda, int scl) {
  Serial.print("  Пины: GPIO");
  Serial.print(sda);
  Serial.print(",");
  Serial.print(scl);
  Serial.println();
  
  // Проверка напряжения
  pinMode(sda, INPUT_PULLUP);
  pinMode(scl, INPUT_PULLUP);
  delay(10);
  
  if (digitalRead(sda) == LOW || digitalRead(scl) == LOW) {
    Serial.println("  ❌ Нет подтяжки 3.3V!");
    return;
  }
  
  // Инициализация I2C
  Wire.begin(sda, scl);
  
  // Сканирование
  Serial.print("  Сканирование... ");
  bool found = false;
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      if (!found) Serial.println("\n  Найдены:");
      Serial.print("    0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found = true;
    }
  }
  
  if (!found) {
    Serial.println("нет устройств");
    Serial.println("  ⚠️  Подключи SSD1306 и проверь еще раз");
  }
  
  Wire.end();
}
