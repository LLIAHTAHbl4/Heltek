/*
 * Heltec WiFi Kit 32 V3.1
 * AS5600 + SHT31 + Батарея
 * Напряжение под нагрузкой: 3.87V
 */

#include "SH1106Wire.h"
#include "Wire.h"
#include "Adafruit_SHT31.h"

// === ПИНЫ ===
#define SDA_PIN   17
#define SCL_PIN   18
#define OLED_VEXT 10
#define OLED_RST  21
#define BAT_ADC   1

// === АДРЕСА ===
#define DISPLAY_ADDR 0x3C
#define AS5600_ADDR  0x36
#define SHT31_ADDR   0x44

// === РЕГИСТРЫ AS5600 ===
#define AS5600_RAW_ANGLE_REG 0x0C
#define AS5600_MAGNITUDE_REG 0x1B

// === КАЛИБРОВКА БАТАРЕИ ===
// Фактические значения:
// - Без нагрузки: 3.9V
// - Под нагрузкой: 3.87V (текущее)
#define ADC_REF_VOLTAGE 3.3
#define ADC_MAX_VALUE 4095
#define VOLTAGE_DIVIDER_RATIO 2.0
#define CALIBRATION_FACTOR 1.0  // Если показывает не 3.87V - поменяй

#define BATTERY_MAX_VOLTAGE 4.2
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_CRITICAL 3.3

// === ОБЪЕКТЫ ===
SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// === ПЕРЕМЕННЫЕ ===
bool as5600Connected = false;
bool sht31Connected = false;
float angle = 0.0;
int magnetPercent = 0;
float temperature = 0.0;
float humidity = 0.0;
float batteryVoltage = 0.0;
int batteryPercent = 0;
bool batteryLow = false;
unsigned long startTime = 0;
unsigned long lastUpdate = 0;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Heltec V3.1 (Батарея 3.87V) ===");
  
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  initDisplay();
  initSensors();
  showStartScreen();
  
  startTime = millis();
}

// ======================= LOOP =======================
void loop() {
  unsigned long now = millis();
  
  if (now - lastUpdate >= 200) {
    lastUpdate = now;
    
    if (as5600Connected) readAS5600();
    if (sht31Connected) readSHT31();
    readBattery();
    
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
  Wire.beginTransmission(AS5600_ADDR);
  as5600Connected = (Wire.endTransmission() == 0);
  Serial.print("AS5600: ");
  Serial.println(as5600Connected ? "OK" : "NO");
  
  sht31Connected = sht31.begin(SHT31_ADDR);
  if (!sht31Connected) sht31Connected = sht31.begin(0x45);
  Serial.print("SHT31: ");
  Serial.println(sht31Connected ? "OK" : "NO");
}

uint16_t readAS5600Register(uint8_t reg) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  
  Wire.requestFrom(AS5600_ADDR, 2);
  if (Wire.available() >= 2) {
    return (Wire.read() << 8) | Wire.read();
  }
  return 0;
}

void readAS5600() {
  uint16_t rawAngle = readAS5600Register(AS5600_RAW_ANGLE_REG);
  angle = (rawAngle * 360.0) / 4096.0;
  
  int magnetStrength = readAS5600Register(AS5600_MAGNITUDE_REG);
  magnetPercent = constrain(map(magnetStrength, 0, 2000, 0, 100), 0, 100);
}

void readSHT31() {
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    temperature = -999;
    humidity = -999;
  }
}

void readBattery() {
  static int readings[5] = {0};
  static int index = 0;
  
  readings[index] = analogRead(BAT_ADC);
  index = (index + 1) % 5;
  
  int average = 0;
  for (int i = 0; i < 5; i++) average += readings[i];
  average /= 5;
  
  // Расчет напряжения: ADC * (3.3V / 4095) * делитель * калибровка
  float rawVoltage = average * (ADC_REF_VOLTAGE / ADC_MAX_VALUE);
  batteryVoltage = rawVoltage * VOLTAGE_DIVIDER_RATIO * CALIBRATION_FACTOR;
  
  // Расчет процента заряда (линейный)
  batteryPercent = mapFloat(batteryVoltage, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE, 0, 100);
  batteryPercent = constrain(batteryPercent, 0, 100);
  
  // Проверка низкого заряда
  batteryLow = (batteryVoltage < BATTERY_CRITICAL);
}

int mapFloat(float x, float in_min, float in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void showStartScreen() {
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "Heltec V3.1");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 30, "Батарея: 3.87V");
  display.drawString(0, 45, "Нагрузка: ВКЛ");
  display.display();
  delay(1500);
}

void updateDisplay() {
  display.clear();
  
  // Вертикальная линия по центру
  display.drawVerticalLine(63, 0, 64);
  
  // ===== ЛЕВАЯ КОЛОНКА (AS5600) =====
  
  // Сила магнита (%)
  display.setFont(ArialMT_Plain_16);
  display.drawString(5, 0, String(magnetPercent) + "%");
  
  // Угол (градусы)
  display.drawString(5, 20, String(angle, 1) + "°");
  
  // Таймер
  display.setFont(ArialMT_Plain_10);
  unsigned long elapsed = (millis() - startTime) / 1000;
  unsigned long h = elapsed / 3600;
  unsigned long m = (elapsed % 3600) / 60;
  unsigned long s = elapsed % 60;
  
  String timer = "";
  if (h > 0) {
    timer += String(h) + ":";
    if (m < 10) timer += "0";
  }
  timer += String(m) + ":";
  if (s < 10) timer += "0";
  timer += String(s);
  
  display.drawString(0, 50, timer);
  
  // ===== ПРАВАЯ КОЛОНКА (SHT31 + Батарея) =====
  
  // Температура
  display.setFont(ArialMT_Plain_16);
  if (temperature != -999) {
    display.drawString(70, 0, String(temperature, 1) + "C");
  } else {
    display.drawString(70, 0, "--C");
  }
  
  // Влажность
  if (humidity != -999) {
    display.drawString(70, 25, String(humidity, 1) + "%");
  } else {
    display.drawString(70, 25, "--%");
  }
  
  // Батарея (напряжение/проценты)
  display.setFont(ArialMT_Plain_10);
  String batStr = String(batteryVoltage, 1) + "V/" + String(batteryPercent) + "%";
  
  // Если низкий заряд - мигаем или меняем цвет (в OLED только инвертировать)
  if (batteryLow) {
    display.fillRect(68, 48, 60, 12);
    display.setColor(BLACK);
    display.drawString(68, 50, batStr);
    display.setColor(WHITE);
  } else {
    display.drawString(68, 50, batStr);
  }
  
  display.display();
}

void printToSerial() {
  Serial.print("Угол: ");
  Serial.print(angle, 1);
  Serial.print("° | MAG: ");
  Serial.print(magnetPercent);
  Serial.print("%");
  
  if (temperature != -999) {
    Serial.print(" | Темп: ");
    Serial.print(temperature, 1);
    Serial.print("C");
  }
  
  if (humidity != -999) {
    Serial.print(" | Влаг: ");
    Serial.print(humidity, 1);
    Serial.print("%");
  }
  
  Serial.print(" | Батарея: ");
  Serial.print(batteryVoltage, 2);
  Serial.print("V (");
  Serial.print(batteryPercent);
  Serial.print("%)");
  
  if (batteryLow) {
    Serial.print(" [НИЗКИЙ ЗАРЯД!]");
  }
  
  Serial.println();
}
