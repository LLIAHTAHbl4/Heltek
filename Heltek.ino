/*
 * Heltec WiFi Kit 32 V3.1
 * ПРОСТОЙ тест пинов I2C с SSD1306
 */

#include "SH1106Wire.h"
#include "Wire.h"

// === ВСТРОЕННЫЙ ДИСПЛЕЙ ===
#define MAIN_SDA  17
#define MAIN_SCL  18
#define OLED_VEXT 10
#define OLED_RST  21
SH1106Wire display(0x3C, MAIN_SDA, MAIN_SCL);

// === ТЕСТИРУЕМЫЕ ПИНЫ ДЛЯ SSD1306 ===
// Попробуй по очереди эти варианты:
// #define TEST_SDA  33  // Вариант 1
// #define TEST_SCL  34  // Вариант 1
// #define TEST_SDA  7   // Вариант 2
// #define TEST_SCL  6   // Вариант 2
#define TEST_SDA  4   // Вариант 3
#define TEST_SCL  5   // Вариант 3

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ТЕСТ ПИНОВ ДЛЯ SSD1306 ===");
  
  initDisplay();
  
  Serial.print("\nТестирую пины: SDA=");
  Serial.print(TEST_SDA);
  Serial.print(", SCL=");
  Serial.println(TEST_SCL);
  
  testI2CPins();
  
  // Бесконечный тест
  while(true) {
    testLoop();
    delay(2000);
  }
}

// ======================= LOOP =======================
void loop() {
  // Не используется - все в setup
}

// ======================= ФУНКЦИИ =======================

void initDisplay() {
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);
  
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  
  Wire.begin(MAIN_SDA, MAIN_SCL);
  display.init();
  display.flipScreenVertically();
  display.clear();
  
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "ТЕСТ ПИНОВ");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, String(TEST_SDA) + "," + String(TEST_SCL));
  display.display();
}

void testI2CPins() {
  // 1. Проверка напряжения
  pinMode(TEST_SDA, INPUT_PULLUP);
  pinMode(TEST_SCL, INPUT_PULLUP);
  delay(10);
  
  int sdaState = digitalRead(TEST_SDA);
  int sclState = digitalRead(TEST_SCL);
  
  Serial.print("Напряжение: SDA=");
  Serial.print(sdaState == HIGH ? "3.3V" : "LOW");
  Serial.print(", SCL=");
  Serial.println(sclState == HIGH ? "3.3V" : "LOW");
  
  // 2. Тест I2C
  Wire.end(); // Отключаем основной Wire
  
  Wire.begin(TEST_SDA, TEST_SCL);
  Wire.setClock(100000);
  
  Serial.println("Сканирование I2C...");
  
  bool found = false;
  for(byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    
    if(error == 0) {
      Serial.print("Найден: 0x");
      if(addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found = true;
      
      if(addr == 0x3C) {
        Serial.println(">>> SSD1306 обнаружен! <<<");
        
        display.clear();
        display.drawString(0, 40, "SSD1306 НАЙДЕН!");
        display.display();
      }
    }
  }
  
  if(!found) {
    Serial.println("Устройств не найдено");
    
    display.clear();
    display.drawString(0, 40, "Нет устройств");
    display.display();
  }
  
  // Возвращаем Wire к основному дисплею
  Wire.end();
  Wire.begin(MAIN_SDA, MAIN_SCL);
}

void testLoop() {
  // Тестовый цикл для осциллографа
  
  // 1. Переключаемся на тестовые пины
  Wire.end();
  Wire.begin(TEST_SDA, TEST_SCL);
  
  // 2. Делаем запрос к адресу 0x3C
  Serial.print("Тест ");
  Serial.print(millis()/1000);
  Serial.println(" сек");
  
  Wire.beginTransmission(0x3C);
  byte error = Wire.endTransmission();
  
  Serial.print("Ответ: ");
  Serial.println(error == 0 ? "Устройство есть" : "Нет ответа");
  
  // 3. Возвращаемся к основному дисплею
  Wire.end();
  Wire.begin(MAIN_SDA, MAIN_SCL);
  
  // 4. Обновляем дисплей
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Тест пинов:");
  display.drawString(0, 15, String(TEST_SDA) + "," + String(TEST_SCL));
  display.drawString(0, 30, "Время: " + String(millis()/1000) + "s");
  display.drawString(0, 45, "Сканирую каждые 2с");
  display.display();
}
