/*
 * Heltec WiFi Kit 32 V3.1 (ESP32-S3)
 * Датчик: AS5600 (Магнитный энкодер)
 * Дисплей: SH1106 128x64
 */

#include "SH1106Wire.h"
#include "AS5600.h"

// === КОНСТАНТЫ ДИСПЛЕЯ ===
#define OLED_VEXT     10    // Питание дисплея (LOW = ВКЛ)
#define OLED_RST      21    // Reset дисплея
#define OLED_SDA      17    // SDA дисплея (НЕ МЕНЯТЬ!)
#define OLED_SCL      18    // SCL дисплея (НЕ МЕНЯТЬ!)
#define OLED_ADDR     0x3C  // Адрес I2C дисплея

// === AS5600 ДАТЧИК ===
// Используем I2C для датчика (адрес 0x36)
#define AS5600_SDA    4     // SDA для AS5600
#define AS5600_SCL    5     // SCL для AS5600

// === ОБЪЕКТЫ ===
SH1106Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);
AS5600 as5600;  // Создаем объект датчика

// Данные датчика
int rawAngle = 0;
int angle = 0;
float degrees = 0.0;
bool magnetDetected = false;
int magnetStrength = 0;
uint8_t status = 0;

// Таймер для обновления
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 500; // 0.5 секунды

// Флаг инициализации датчика
bool as5600Found = false;

// ======================= SETUP =======================
void setup() {
  // Инициализация Serial
  Serial.begin(115200);
  Serial.println("\n=== Heltec WiFi Kit 32 V3.1 ===");
  Serial.println("Датчик: AS5600 (Магнитный энкодер)");
  
  // Инициализация дисплея
  initDisplay();
  
  // Инициализация датчика AS5600
  initAS5600();
  
  // Стартовый экран
  showStartScreen();
}

// ======================= LOOP =======================
void loop() {
  unsigned long currentMillis = millis();
  
  // Обновление данных каждые 0.5 секунды
  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;
    
    // Чтение данных с датчика
    readAS5600();
    
    // Вывод в Serial Monitor
    printToSerial();
    
    // Вывод на дисплей
    updateDisplay();
  }
  
  delay(50);
}

// ======================= ФУНКЦИИ =======================

// Инициализация дисплея
void initDisplay() {
  Serial.println("Инициализация дисплея...");
  
  // Включение питания дисплея
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);
  
  // Сброс дисплея
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  delay(50);
  
  // Инициализация дисплея
  Wire.begin(OLED_SDA, OLED_SCL);
  display.init();
  display.flipScreenVertically(); // ОБЯЗАТЕЛЬНО!
  display.clear();
  
  Serial.println("Дисплей инициализирован!");
}

// Инициализация датчика AS5600
void initAS5600() {
  Serial.println("Поиск датчика AS5600...");
  
  // Инициализация I2C для датчика (Wire1)
  Wire1.begin(AS5600_SDA, AS5600_SCL);
  Wire1.setClock(100000);
  
  // Инициализация датчика с использованием Wire1
  as5600.begin(4);  // Не используем направляющий пин
  
  // Устанавливаем I2C для датчика
  as5600.setWire(&Wire1);
  
  // Проверка подключения
  if (as5600.isConnected()) {
    as5600Found = true;
    Serial.println("Датчик AS5600 найден!");
    Serial.print("Адрес I2C: 0x");
    Serial.println(as5600.getAddress(), HEX);
  } else {
    as5600Found = false;
    Serial.println("ОШИБКА: Датчик AS5600 не найден!");
    Serial.println("Проверьте подключение:");
    Serial.println("SDA -> GPIO4");
    Serial.println("SCL -> GPIO5");
    Serial.println("VCC -> 3.3V или 5V");
    Serial.println("GND -> GND");
    Serial.println("Адрес I2C: 0x36");
  }
}

// Чтение данных с AS5600
void readAS5600() {
  if (as5600Found) {
    // Чтение сырого значения угла (0-4095)
    rawAngle = as5600.readAngle();
    
    // Чтение угла (0-4095)
    angle = as5600.readAngle();
    
    // Преобразование в градусы (0-360)
    degrees = (angle * 360.0) / 4096.0;
    
    // Проверка наличия магнита
    magnetDetected = as5600.detectMagnet();
    
    // Чтение силы магнита
    magnetStrength = as5600.readMagnitude();
    
    // Чтение статуса
    status = as5600.readStatus();
  }
}

// Вывод данных в Serial Monitor
void printToSerial() {
  Serial.println("\n=== Данные AS5600 ===");
  
  if (as5600Found) {
    Serial.print("Сырое значение: ");
    Serial.println(rawAngle);
    
    Serial.print("Угол: ");
    Serial.print(angle);
    Serial.println(" ед.");
    
    Serial.print("Градусы: ");
    Serial.print(degrees, 1);
    Serial.println("°");
    
    Serial.print("Магнит: ");
    if (magnetDetected) {
      Serial.println("ОБНАРУЖЕН");
    } else {
      Serial.println("НЕ ОБНАРУЖЕН");
    }
    
    // Дополнительная информация
    Serial.print("Сила магнита: ");
    Serial.println(magnetStrength);
    
    Serial.print("Статус: 0x");
    Serial.println(status, HEX);
    
    if (magnetStrength < 100) {
      Serial.println("ВНИМАНИЕ: Слабое магнитное поле!");
    }
  } else {
    Serial.println("Датчик не найден!");
  }
  
  Serial.print("Время: ");
  Serial.print(millis() / 1000);
  Serial.println(" сек");
  Serial.println("===================");
}

// Обновление дисплея
void updateDisplay() {
  display.clear();
  
  // Заголовок
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Heltec V3.1 - AS5600");
  
  if (as5600Found) {
    if (magnetDetected) {
      // Основной угол большими цифрами
      display.setFont(ArialMT_Plain_24);
      display.drawString(0, 15, String(degrees, 1) + char(247));
      
      // Сырое значение
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 45, "RAW: " + String(rawAngle));
      
      // Информация о магните
      display.drawString(0, 55, "MAG: " + String(magnetStrength));
      
      // Статус
      display.drawString(70, 45, "OK");
      
      // Индикатор силы магнита
      if (magnetStrength < 100) {
        display.drawString(70, 55, "WEAK");
      }
    } else {
      // Магнит не обнаружен
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 20, "NO MAGNET");
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 45, "Place magnet near");
      display.drawString(0, 55, "the sensor");
    }
  } else {
    // Датчик не найден
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 20, "AS5600 ERROR");
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 45, "Check connection");
  }
  
  // Время работы
  display.setFont(ArialMT_Plain_10);
  display.drawString(85, 0, String(millis() / 1000) + "s");
  
  display.display();
}

// Стартовый экран
void showStartScreen() {
  display.clear();
  
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 0, "Heltec V3.1");
  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "Sensor: AS5600");
  
  if (as5600Found) {
    display.drawString(0, 40, "Status: FOUND");
    display.drawString(0, 50, "Addr: 0x36");
  } else {
    display.drawString(0, 40, "Status: ERROR");
    display.drawString(0, 50, "Check wiring");
  }
  
  display.drawString(0, 60, "Starting...");
  
  display.display();
  delay(2000);
}
