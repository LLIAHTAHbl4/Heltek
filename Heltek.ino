/*

Heltec WiFi Kit 32 V3.1

AS5600 (магнитный энкодер) + SHT31 (температура/влажность) + Батарея

Оба на пинах 17,18
*/


#include "SH1106Wire.h"
#include "Wire.h"
#include "Adafruit_SHT31.h"

// === ПИНЫ ===
#define SDA_PIN   17  // Единственные рабочие I2C пины
#define SCL_PIN   18
#define OLED_VEXT 10  // Питание дисплея
#define OLED_RST  21  // Reset дисплея
#define BAT_ADC   1   // Пин измерения батареи (GPIO1 = ADC1_CH0)

// === АДРЕСА ===
#define DISPLAY_ADDR 0x3C
#define AS5600_ADDR  0x36
#define SHT31_ADDR   0x44  // SHT31 адрес (0x44 или 0x45)

// === РЕГИСТРЫ AS5600 ===
#define AS5600_RAW_ANGLE_REG 0x0C
#define AS5600_ANGLE_REG     0x0E
#define AS5600_STATUS_REG    0x0B
#define AS5600_MAGNITUDE_REG 0x1B

// === КАЛИБРОВКА АККУМУЛЯТОРА ===
#define BATTERY_MAX_VOLTAGE 4.2  // Максимальное напряжение Li-ion банки
#define BATTERY_MIN_VOLTAGE 3.0  // Минимальное напряжение Li-ion банки
#define VOLTAGE_DIVIDER_RATIO 2.0 // Делитель напряжения (если есть)

// === ОБЪЕКТЫ ===
SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// === ПЕРЕМЕННЫЕ ===
bool as5600Connected = false;
bool sht31Connected = false;
float currentAngle = 0.0;
int magnetStrength = 0;
int magnetPercent = 0;
float temperature = 0.0;
float humidity = 0.0;
float batteryVoltage = 0.0;
int batteryPercent = 0;
unsigned long startTime = 0;
unsigned long lastUpdate = 0;
unsigned long lastBatteryRead = 0;

// ======================= SETUP =======================
void setup() {
Serial.begin(115200);
Serial.println("\n=== Heltec V3.1 - AS5600 + SHT31 + Батарея ===");

// Настройка АЦП для батареи
analogReadResolution(12); // 12 бит (0-4095)
analogSetAttenuation(ADC_11db); // Диапазон до 3.9V

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
  
// Чтение батареи каждые 2 секунды  
if (now - lastBatteryRead >= 2000) {  
  lastBatteryRead = now;  
  readBattery();  
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
// Пробуем альтернативный адрес
if (sht31.begin(0x45)) {
sht31Connected = true;
Serial.println("SHT31 найден (0x45)");
} else {
sht31Connected = false;
Serial.println("SHT31 не найден");
}
}
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

// Читаем силу магнита (0-4095)
magnetStrength = readAS5600Register(AS5600_MAGNITUDE_REG);

// Конвертируем в проценты (0-100%)
// AS5600: 0-4095, нормальные значения ~100-2000
magnetPercent = map(magnetStrength, 0, 2000, 0, 100);
if (magnetPercent < 0) magnetPercent = 0;
if (magnetPercent > 100) magnetPercent = 100;
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

void readBattery() {
// Чтение напряжения с АЦП
int adcValue = analogRead(BAT_ADC);

// Конвертация в напряжение (для ESP32-S3, 12 бит, 3.3V)
// Коэффициент зависит от конкретной платы Heltec
float voltage = adcValue * (3.3 / 4095.0);

// Если есть делитель напряжения, учитываем его
batteryVoltage = voltage * VOLTAGE_DIVIDER_RATIO;

// Расчет процента заряда (линейная аппроксимация)
batteryPercent = mapFloat(batteryVoltage, BATTERY_MIN_VOLTAGE, BATTERY_MAX_VOLTAGE, 0, 100);
if (batteryPercent < 0) batteryPercent = 0;
if (batteryPercent > 100) batteryPercent = 100;

Serial.print("Батарея: ");
Serial.print(batteryVoltage, 2);
Serial.print("V (");
Serial.print(batteryPercent);
Serial.println("%)");
}

int mapFloat(float x, float in_min, float in_max, int out_min, int out_max) {
return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void showStartScreen() {
display.clear();
display.setFont(ArialMT_Plain_16);
display.drawString(10, 0, "Heltec V3.1");

display.setFont(ArialMT_Plain_10);
display.drawString(0, 25, "AS5600: " + String(as5600Connected ? "OK" : "NO"));
display.drawString(0, 40, "SHT31: " + String(sht31Connected ? "OK" : "NO"));
display.drawString(0, 55, "Батарея: тест...");

display.display();
delay(2000);
}

void updateDisplay() {
display.clear();

// Вертикальная разделительная линия по центру
display.drawVerticalLine(63, 0, 64);

// ===== ЛЕВАЯ КОЛОНКА (AS5600) =====

// 1. Сила магнита в процентах (верхняя строка) - шрифт 16
display.setFont(ArialMT_Plain_16);
String magStr = String(magnetPercent) + "%";
display.drawString(5, 0, magStr);

// 2. Угол в градусах (средняя строка) - ТЕПЕРЬ ТОТ ЖЕ ШРИФТ 16
String angleStr = String(currentAngle, 1) + "°";

// Центрируем угол в зависимости от длины
int angleX = 0;
if (currentAngle < 10) angleX = 15;
else if (currentAngle < 100) angleX = 5;

display.drawString(angleX, 20, angleStr);

// 3. Таймер (нижняя строка) - шрифт 10
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

display.setFont(ArialMT_Plain_10);
display.drawString(0, 50, timerStr);

// ===== ПРАВАЯ КОЛОНКА (SHT31 + Батарея) =====

// 1. Температура (верхняя строка) - шрифт 16
display.setFont(ArialMT_Plain_16);
if (temperature != -999) {
String tempStr = String(temperature, 1) + "°C";
display.drawString(70, 0, tempStr);
} else {
display.drawString(70, 0, "--°C");
}

// 2. Влажность (средняя строка) - шрифт 16
if (humidity != -999) {
String humStr = String(humidity, 1) + "%";
display.drawString(70, 25, humStr);
} else {
display.drawString(70, 25, "--%");
}

// 3. Батарея (нижняя строка) - шрифт 10 (как таймер)
display.setFont(ArialMT_Plain_10);
String batStr = String(batteryVoltage, 1) + "V/" + String(batteryPercent) + "%";
display.drawString(68, 50, batStr);

display.display();
}

void printToSerial() {
Serial.print("Угол: ");
Serial.print(currentAngle, 1);
Serial.print("° | MAG: ");
Serial.print(magnetPercent);
Serial.print("% (");
Serial.print(magnetStrength);
Serial.print(")");

if (sht31Connected && temperature != -999 && humidity != -999) {
Serial.print(" | Темп: ");
Serial.print(temperature, 1);
Serial.print("°C | Влаг: ");
Serial.print(humidity, 1);
Serial.print("%");
}

Serial.print(" | Батарея: ");
Serial.print(batteryVoltage, 2);
Serial.print("V (");
Serial.print(batteryPercent);
Serial.println("%)");
}
Пока ничего не пеши в чат, добавлю ещо информации, потом скажу когда можешь отвечать
