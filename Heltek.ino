/*
 * Heltec WiFi Kit 32 V3.1
 * AS5600 (магнитный энкодер) + SHT31 (температура/влажность)
 * Оба на пинах 17,18
 */

#include "SH1106Wire.h"
#include "Wire.h"
#include "Adafruit_SHT31.h"

// === ПИНЫ ===
#define SDA_PIN   17  // Единственные рабочие I2C пины
#define SCL_PIN   18
#define OLED_VEXT 10  // Питание дисплея
#define OLED_RST  21  // Reset дисплея

// === АДРЕСА ===
#define DISPLAY_ADDR 0x3C
#define AS5600_ADDR  0x36
#define SHT31_ADDR   0x44  // SHT31 адрес (0x44 или 0x45)

// === РЕГИСТРЫ AS5600 ===
#define AS5600_RAW_ANGLE_REG 0x0C
#define AS5600_ANGLE_REG     0x0E
#define AS5600_STATUS_REG    0x0B
#define AS5600_MAGNITUDE_REG 0x1B

// === ОБЪЕКТЫ ===
SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// === ПЕРЕМЕННЫЕ ===
bool as5600Connected = false;
bool sht31Connected = false;
float currentAngle = 0.0;
bool magnetDetected = false;
int magnetStrength = 0;
float temperature = 0.0;
float humidity = 0.0;
unsigned long startTime = 0;
unsigned long lastUpdate = 0;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Heltec V3.1 - AS5600 + SHT31 ===");
  
  initDisplay();
  initSensors();
  showStartScreen();
  
  startTime = millis(); // Запуск таймера
}

// ======================= LOOP =======================
void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate >= 200) { // 5 раз в секунду
    lastUpdate = now;
    
    if (as5600Connected) {
      readAS5600();
    }
    
    if (sht31Connected) {
      readSHT31();
    }
    
    updateDisplay();
    printToSerial();
  }
  
  delay(10);
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

void initSensors() {
  Serial.println("Инициализация датчиков...");
  
  // Проверка AS5600
  Wire.beginTransmission(AS5600_ADDR);
  if (Wire.endTransmission() == 0) {
    as5600Connected = true;
    Serial.println("AS5600 найден (0x36)");
  } else {
    as5600Connected = false;
    Serial.println("AS5600 не найден");
  }
  
  // Проверка SHT31
  if (sht31.begin(SHT31_ADDR)) {
    sht31Connected = true;
    Serial.println("SHT31 найден (0x44)");
  } else {
    sht31Connected = false;
    Serial.println("SHT31 не найден");
  }
  
  // Сканирование всех устройств
  scanI2C();
}

void scanI2C() {
  Serial.println("Сканирование I2C шины:");
  int found = 0;
  
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("  0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      
      if (addr == DISPLAY_ADDR) Serial.print(" (Дисплей)");
      if (addr == AS5600_ADDR) Serial.print(" (AS5600)");
      if (addr == SHT31_ADDR) Serial.print(" (SHT31)");
      
      Serial.println();
      found++;
    }
  }
  
  Serial.print("Всего устройств: ");
  Serial.println(found);
}

uint16_t readAS5600Register(uint8_t reg) {
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

uint8_t readAS5600Status() {
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
  uint16_t rawAngle = readAS5600Register(AS5600_RAW_ANGLE_REG);
  
  // Конвертируем в градусы (0-360)
  currentAngle = (rawAngle * 360.0) / 4096.0;
  
  // Читаем статус магнита
  uint8_t status = readAS5600Status();
  magnetDetected = (status & 0x20) != 0; // Бит 5: MD (Magnet Detected)
  
  // Читаем силу магнита
  magnetStrength = readAS5600Register(AS5600_MAGNITUDE_REG);
}

void readSHT31() {
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  
  // Проверка на ошибки чтения
  if (isnan(temperature) || isnan(humidity)) {
    temperature = -999;
    humidity = -999;
  }
}

void showStartScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "Heltec V3.1");
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "AS5600: " + String(as5600Connected ? "OK" : "NO"));
  display.drawString(0, 40, "SHT31: " + String(sht31Connected ? "OK" : "NO"));
  display.drawString(0, 55, "Загрузка...");
  
  display.display();
  delay(2000);
}

void updateDisplay() {
  display.clear();
  
  // Вертикальная разделительная линия по центру
  display.drawVerticalLine(63, 0, 64);
  
  // ===== ЛЕВАЯ КОЛОНКА (AS5600) =====
  
  // 1. Сила магнита (верхняя строка)
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "MAG:");
  display.drawString(30, 0, String(magnetStrength));
  
  // 2. Угол в градусах (средняя строка)
  display.setFont(ArialMT_Plain_16);
  String angleStr = String(currentAngle, 1) + "°";
  int angleX = 0;
  if (currentAngle < 100) angleX = 10;
  display.drawString(angleX, 15, angleStr);
  
  // Индикатор магнита
  display.setFont(ArialMT_Plain_10);
  if (magnetDetected) {
    display.drawString(0, 35, "M:ON");
  } else {
    display.drawString(0, 35, "M:OFF");
  }
  
  // 3. Таймер (нижняя строка)
  unsigned long elapsed = (millis() - startTime) / 1000; // секунды
  unsigned long hours = elapsed / 3600;
  unsigned long minutes = (elapsed % 3600) / 60;
  unsigned long seconds = elapsed % 60;
  
  String timerStr = "";
  if (hours > 0) {
    timerStr += String(hours) + ":";
    if (minutes < 10) timerStr += "0";
  }
  timerStr += String(minutes) + ":";
  if (seconds < 10) timerStr += "0";
  timerStr += String(seconds);
  
  display.drawString(0, 50, timerStr);
  
  // ===== ПРАВАЯ КОЛОНКА (SHT31) =====
  
  // 1. Температура (верхняя строка)
  display.setFont(ArialMT_Plain_16);
  if (temperature != -999) {
    String tempStr = String(temperature, 1) + "C";
    display.drawString(70, 0, tempStr);
  } else {
    display.drawString(70, 0, "--C");
  }
  
  // 2. Влажность (нижняя строка)
  display.setFont(ArialMT_Plain_16);
  if (humidity != -999) {
    String humStr = String(humidity, 1) + "%";
    display.drawString(70, 25, humStr);
  } else {
    display.drawString(70, 25, "--%");
  }
  
  // Статус датчиков внизу справа
  display.setFont(ArialMT_Plain_10);
  display.drawString(70, 50, "A:" + String(as5600Connected ? "1" : "0"));
  display.drawString(90, 50, "S:" + String(sht31Connected ? "1" : "0"));
  
  display.display();
}

void printToSerial() {
  Serial.print("Угол: ");
  Serial.print(currentAngle, 1);
  Serial.print("° | Магнит: ");
  Serial.print(magnetDetected ? "ДА" : "НЕТ");
  Serial.print("(");
  Serial.print(magnetStrength);
  Serial.print(")");
  
  if (sht31Connected && temperature != -999 && humidity != -999) {
    Serial.print(" | Темп: ");
    Serial.print(temperature, 1);
    Serial.print("C | Влаг: ");
    Serial.print(humidity, 1);
    Serial.print("%");
  }
  
  Serial.println();
}
