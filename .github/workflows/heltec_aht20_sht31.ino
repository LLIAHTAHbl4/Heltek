/*
 * Heltec WiFi Kit 32 V3.1 (ESP32-S3)
 * Датчики: AHT20 и SHT31
 * Дисплей: SH1106 128x64 (OLED)
 * 
 * Подключение:
 * AHT20: SDA -> GPIO4, SCL -> GPIO5, VCC -> 3.3V, GND -> GND
 * SHT31: SDA -> GPIO33, SCL -> GPIO34, VCC -> 3.3V, GND -> GND
 */

#include "SH1106Wire.h"
#include "Adafruit_AHTX0.h"
#include "Adafruit_SHT31.h"

// === КОНСТАНТЫ ДИСПЛЕЯ ===
#define OLED_VEXT     10    // Питание дисплея (LOW = ВКЛ)
#define OLED_RST      21    // Reset дисплея
#define OLED_SDA      17    // SDA дисплея (НЕ МЕНЯТЬ!)
#define OLED_SCL      18    // SCL дисплея (НЕ МЕНЯТЬ!)
#define OLED_ADDR     0x3C  // Адрес I2C дисплея

// === I2C ДЛЯ ДАТЧИКОВ ===
// AHT20 на первом I2C (пины 4,5)
#define I2C_AHT_SDA   4
#define I2C_AHT_SCL   5

// SHT31 на втором I2C (пины 33,34)
#define I2C_SHT_SDA   33
#define I2C_SHT_SCL   34

// === ОБЪЕКТЫ ===
SH1106Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);
Adafruit_AHTX0 aht;
Adafruit_SHT31 sht = Adafruit_SHT31();

// Переменные для данных
sensors_event_t humidity_aht, temp_aht;
float temp_sht = 0;
float hum_sht = 0;

// Время последнего обновления
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000; // 2 секунды

// ======================= SETUP =======================
void setup() {
  // Инициализация Serial
  Serial.begin(115200);
  Serial.println("\n=== Heltec WiFi Kit 32 V3.1 ===");
  Serial.println("Датчики: AHT20 + SHT31");
  
  // ===== ИНИЦИАЛИЗАЦИЯ ДИСПЛЕЯ =====
  initDisplay();
  
  // ===== ИНИЦИАЛИЗАЦИЯ I2C ДЛЯ ДАТЧИКОВ =====
  initI2CForSensors();
  
  // ===== ИНИЦИАЛИЗАЦИЯ ДАТЧИКОВ =====
  initSensors();
  
  // Вывод начального сообщения
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Heltec V3.1");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, "AHT20 + SHT31");
  display.drawString(0, 35, "Готов к работе");
  display.display();
  
  delay(2000);
}

// ======================= LOOP =======================
void loop() {
  unsigned long currentMillis = millis();
  
  // Обновление данных каждые 2 секунды
  if (currentMillis - lastUpdate >= updateInterval) {
    lastUpdate = currentMillis;
    
    // Чтение данных с датчиков
    readSensors();
    
    // Вывод в Serial Monitor
    printToSerial();
    
    // Вывод на дисплей
    updateDisplay();
  }
  
  delay(100);
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
  
  // Инициализация I2C для дисплея
  Wire.begin(OLED_SDA, OLED_SCL);
  display.init();
  display.flipScreenVertically(); // ОБЯЗАТЕЛЬНО!
  display.clear();
  
  Serial.println("Дисплей инициализирован!");
}

// Инициализация I2C для датчиков
void initI2CForSensors() {
  Serial.println("Настройка I2C для датчиков...");
  
  // Настройка I2C для AHT20 (пины 4,5)
  Wire1.begin(I2C_AHT_SDA, I2C_AHT_SCL);
  Wire1.setClock(100000);
  
  Serial.println("I2C настроены: AHT20(4,5), SHT31(33,34)");
}

// Инициализация датчиков
void initSensors() {
  Serial.println("Поиск датчиков...");
  
  // Инициализация AHT20
  if (aht.begin(&Wire1)) {
    Serial.println("AHT20 найден!");
  } else {
    Serial.println("AHT20 не найден! Проверьте подключение.");
    displayError("AHT20 не найден!");
  }
  
  // Инициализация SHT31
  if (sht.begin(0x44, &Wire)) {
    Serial.println("SHT31 найден!");
  } else {
    Serial.println("SHT31 не найден! Проверьте подключение.");
    displayError("SHT31 не найден!");
  }
}

// Чтение данных с датчиков
void readSensors() {
  // Чтение AHT20
  if (aht.getEvent(&humidity_aht, &temp_aht)) {
    // Данные уже в переменных
  } else {
    Serial.println("Ошибка чтения AHT20!");
  }
  
  // Чтение SHT31
  temp_sht = sht.readTemperature();
  hum_sht = sht.readHumidity();
  
  // Проверка ошибок SHT31
  if (isnan(temp_sht) || isnan(hum_sht)) {
    Serial.println("Ошибка чтения SHT31!");
    temp_sht = -999;
    hum_sht = -999;
  }
}

// Вывод данных в Serial Monitor
void printToSerial() {
  Serial.println("\n=== Данные с датчиков ===");
  Serial.print("AHT20: ");
  Serial.print(temp_aht.temperature);
  Serial.print("°C, ");
  Serial.print(humidity_aht.relative_humidity);
  Serial.println("%");
  
  Serial.print("SHT31: ");
  Serial.print(temp_sht);
  Serial.print("°C, ");
  Serial.print(hum_sht);
  Serial.println("%");
  Serial.println("========================");
}

// Обновление дисплея
void updateDisplay() {
  display.clear();
  
  // Заголовок
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Heltec V3.1");
  
  // Данные AHT20
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "AHT20:");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 25, String(temp_aht.temperature, 1) + "C");
  display.drawString(70, 25, String(humidity_aht.relative_humidity, 1) + "%");
  
  // Данные SHT31
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 45, "SHT31:");
  display.setFont(ArialMT_Plain_16);
  
  if (temp_sht != -999) {
    display.drawString(0, 55, String(temp_sht, 1) + "C");
    display.drawString(70, 55, String(hum_sht, 1) + "%");
  } else {
    display.drawString(0, 55, "ERROR");
  }
  
  display.display();
}

// Отображение ошибки на дисплее
void displayError(String message) {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 20, "ОШИБКА:");
  display.drawString(0, 35, message);
  display.display();
  delay(3000);
}
