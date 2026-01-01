/*
 * Heltec V3.1 - Один дисплей + AS5600 (имитация)
 * Показывает угол и время
 */

#include "SH1106Wire.h"
#include "Wire.h"

// === ПИНЫ ===
#define SDA_PIN   17
#define SCL_PIN   18
#define OLED_VEXT 10
#define OLED_RST  21

// === АДРЕСА ===
#define DISPLAY_ADDR 0x3C
#define AS5600_ADDR  0x36

SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);

// === ПЕРЕМЕННЫЕ ===
bool as5600Connected = false;
float currentAngle = 0.0;
unsigned long lastUpdate = 0;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Heltec V3.1 - Дисплей + AS5600 ===");
  
  initDisplay();
  scanI2C();
  showStartScreen();
}

// ======================= LOOP =======================
void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate >= 100) { // 10 раз в секунду
    lastUpdate = now;
    
    if (as5600Connected) {
      // Имитация чтения AS5600
      simulateAS5600();
    }
    
    updateDisplay();
  }
  
  delay(1);
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
  delay(50);
  
  Wire.begin(SDA_PIN, SCL_PIN);
  display.init();
  display.flipScreenVertically();
  display.clear();
}

void scanI2C() {
  Serial.println("Сканирование I2C...");
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Найден: 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      
      if (addr == DISPLAY_ADDR) Serial.print(" (Дисплей)");
      if (addr == AS5600_ADDR) {
        Serial.print(" (AS5600!)");
        as5600Connected = true;
      }
      
      Serial.println();
    }
  }
}

void showStartScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "Heltec V3.1");
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "Дисплей: 0x3C");
  
  if (as5600Connected) {
    display.drawString(0, 40, "AS5600: Найден");
    display.drawString(0, 55, "Адрес: 0x36");
  } else {
    display.drawString(0, 40, "AS5600: Нет");
    display.drawString(0, 55, "Имитация данных");
  }
  
  display.display();
  delay(2000);
}

void simulateAS5600() {
  // Имитация вращения от 0 до 360 градусов
  static float angle = 0.0;
  angle += 1.2; // Скорость вращения
  
  if (angle > 360.0) {
    angle = 0.0;
  }
  
  currentAngle = angle;
}

void updateDisplay() {
  unsigned long now = millis();
  unsigned long seconds = now / 1000;
  unsigned long milliseconds = now % 1000;
  
  // Время: секунды:миллисекунды
  String timeStr = String(seconds) + ":";
  if (milliseconds < 100) timeStr += "0";
  if (milliseconds < 10) timeStr += "0";
  timeStr += String(milliseconds);
  
  display.clear();
  
  // Верхняя строка - время
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Время: " + timeStr);
  
  // Большой угол
  display.setFont(ArialMT_Plain_24);
  String angleStr = String(currentAngle, 1) + "°";
  display.drawString(10, 15, angleStr);
  
  // Статус внизу
  display.setFont(ArialMT_Plain_10);
  if (as5600Connected) {
    display.drawString(0, 45, "AS5600: Подключен");
    display.drawString(0, 55, "Реальные данные");
  } else {
    display.drawString(0, 45, "AS5600: Не найден");
    display.drawString(0, 55, "Имитация: " + angleStr);
  }
  
  display.display();
}
