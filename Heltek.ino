/*
 * Тест физических пинов 14,15 (GPIO4,5) или 12,13 (GPIO7,6)
 * Подключи SSD1306 к физическим пинам 14(SDA),15(SCL)
 */

#include "Wire.h"

// === ВЫБЕРИ ВАРИАНТ ПОДКЛЮЧЕНИЯ ===
// Вариант 1: К физическим пинам 14,15
#define TEST_SDA 4  // GPIO4 (физический пин 15)
#define TEST_SCL 5  // GPIO5 (физический пин 14)

// Вариант 2: К физическим пинам 12,13
// #define TEST_SDA 7  // GPIO7 (физический пин 12)
// #define TEST_SCL 6  // GPIO6 (физический пин 13)

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ТЕСТ ФИЗИЧЕСКИХ ПИНОВ ===");
  
  Serial.print("Тестирую GPIO");
  Serial.print(TEST_SDA);
  Serial.print(",");
  Serial.print(TEST_SCL);
  Serial.print(" (физические пины: ");
  
  // Определяем физические пины по GPIO
  if(TEST_SDA == 4 && TEST_SCL == 5) {
    Serial.println("14,15)");
  } else if(TEST_SDA == 7 && TEST_SCL == 6) {
    Serial.println("12,13)");
  } else {
    Serial.println("?)");
  }
  
  // Тест 1: Проверка напряжения
  testVoltage();
  
  // Тест 2: Сканирование I2C
  testI2C();
  
  Serial.println("\nКаждые 2 секунды буду сканировать I2C...");
  Serial.println("Смотри импульсы на осциллографе!");
}

void loop() {
  delay(2000);
  scanI2C();
}

void testVoltage() {
  Serial.println("\n1. ПРОВЕРКА НАПРЯЖЕНИЯ:");
  
  pinMode(TEST_SDA, INPUT_PULLUP);
  pinMode(TEST_SCL, INPUT_PULLUP);
  delay(10);
  
  int sda = digitalRead(TEST_SDA);
  int scl = digitalRead(TEST_SCL);
  
  Serial.print("  SDA: ");
  Serial.print(sda == HIGH ? "3.3V ✅" : "LOW ⚠️");
  Serial.print("  |  SCL: ");
  Serial.println(scl == HIGH ? "3.3V ✅" : "LOW ⚠️");
  
  if(sda == LOW || scl == LOW) {
    Serial.println("  ❌ Нет подтяжки! Проверь подключение.");
  }
}

void testI2C() {
  Serial.println("\n2. ИНИЦИАЛИЗАЦИЯ I2C:");
  
  Wire.begin(TEST_SDA, TEST_SCL);
  Wire.setClock(100000);
  
  Serial.print("  I2C на пинах ");
  Serial.print(TEST_SDA);
  Serial.print(",");
  Serial.print(TEST_SCL);
  Serial.println(" (100 kHz)");
}

void scanI2C() {
  Serial.print("\n[");
  Serial.print(millis()/1000);
  Serial.print("с] Сканирование I2C: ");
  
  bool found = false;
  for(byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    
    if(error == 0) {
      if(!found) Serial.print("Найдены: ");
      Serial.print("0x");
      if(addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print(" ");
      found = true;
      
      // Определяем тип устройства
      if(addr == 0x3C || addr == 0x3D) {
        Serial.print("(OLED) ");
      } else if(addr == 0x68) {
        Serial.print("(MPU6050) ");
      } else if(addr == 0x76 || addr == 0x77) {
        Serial.print("(BMP280) ");
      } else if(addr == 0x36) {
        Serial.print("(AS5600!) ");
      }
    }
  }
  
  if(!found) {
    Serial.print("Нет устройств");
    
    // Проверяем, работает ли шина вообще
    Wire.beginTransmission(0x00);
    byte error = Wire.endTransmission();
    if(error == 2) {
      Serial.print(" (шина работает, нет устройств)");
    } else {
      Serial.print(" (проблема с шиной!)");
    }
  }
  
  Serial.println();
  
  // Для осциллографа: дополнительный тестовый запрос
  Serial.print("  Тест адреса 0x3C: ");
  Wire.beginTransmission(0x3C);
  byte err = Wire.endTransmission();
  switch(err) {
    case 0: Serial.println("Устройство отвечает!"); break;
    case 2: Serial.println("Нет ответа (NACK)"); break;
    default: Serial.print("Ошибка "); Serial.println(err); break;
  }
}
