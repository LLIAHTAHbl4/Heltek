/*
 * Heltec V3.1 - Реальное чтение AS5600
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

// === РЕГИСТРЫ AS5600 ===
#define AS5600_RAW_ANGLE_REG 0x0C
#define AS5600_ANGLE_REG     0x0E
#define AS5600_STATUS_REG    0x0B
#define AS5600_MAGNITUDE_REG 0x1B

SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);

// === ПЕРЕМЕННЫЕ ===
bool as5600Connected = false;
float currentAngle = 0.0;
bool magnetDetected = false;
int magnetStrength = 0;
unsigned long lastUpdate = 0;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Heltec V3.1 - AS5600 реальный ===");
  
  initDisplay();
  checkAS5600();
  showStartScreen();
}

// ======================= LOOP =======================
void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate >= 100) { // 10 раз в секунду
    lastUpdate = now;
    
    if (as5600Connected) {
      readAS5600();
    }
    
    updateDisplay();
    printToSerial();
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

void checkAS5600() {
  Wire.beginTransmission(AS5600_ADDR);
  if (Wire.endTransmission() == 0) {
    as5600Connected = true;
    Serial.println("AS5600 найден!");
  } else {
    as5600Connected = false;
    Serial.println("AS5600 не найден!");
  }
}

uint16_t readRegister(uint8_t reg) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() >= 2) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();
    return (highByte << 8) | lowByte;
  }
  return 0;
}

uint8_t readStatus() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_STATUS_REG);
  Wire.endTransmission(false);
  
  Wire.requestFrom(AS5600_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void readAS5600() {
  // Читаем сырой угол (0-4095)
  uint16_t rawAngle = readRegister(AS5600_RAW_ANGLE_REG);
  
  // Конвертируем в градусы (0-360)
  currentAngle = (rawAngle * 360.0) / 4096.0;
  
  // Читаем статус магнита
  uint8_t status = readStatus();
  magnetDetected = (status & 0x20) != 0; // Бит 5: MD (Magnet Detected)
  
  // Читаем силу магнита
  magnetStrength = readRegister(AS5600_MAGNITUDE_REG);
}

void showStartScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "AS5600 Тест");
  
  display.setFont(ArialMT_Plain_10);
  if (as5600Connected) {
    display.drawString(0, 25, "Датчик: Найден");
    display.drawString(0, 40, "Адрес: 0x36");
    display.drawString(0, 55, "Поднеси магнит");
  } else {
    display.drawString(0, 25, "Датчик: Не найден");
    display.drawString(0, 40, "Проверь подключение");
    display.drawString(0, 55, "Адрес: 0x36");
  }
  
  display.display();
  delay(2000);
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
  
  // Статус магнита
  display.setFont(ArialMT_Plain_10);
  if (magnetDetected) {
    display.drawString(0, 45, "Магнит: ДА");
    display.drawString(50, 45, "Сила: " + String(magnetStrength));
  } else {
    display.drawString(0, 45, "Магнит: НЕТ");
  }
  
  // Индикатор подключения
  if (as5600Connected) {
    display.drawString(0, 55, "AS5600: OK");
  } else {
    display.drawString(0, 55, "AS5600: НЕТ");
  }
  
  display.display();
}

void printToSerial() {
  Serial.print("Угол: ");
  Serial.print(currentAngle, 1);
  Serial.print("° | Магнит: ");
  Serial.print(magnetDetected ? "ДА" : "НЕТ");
  Serial.print(" | Сила: ");
  Serial.print(magnetStrength);
  Serial.print(" | RAW: ");
  Serial.println(int(currentAngle * 4096 / 360));
}
