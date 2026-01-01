/*
 * Heltec WiFi Kit 32 V3.1
 * Два дисплея: встроенный SH1106 + дополнительный SSD1306
 * Работает с библиотекой версии 4.6.1
 */

#include "SH1106Wire.h"  // Для встроенного дисплея
#include "SSD1306Wire.h" // Для дополнительного дисплея
#include "Wire.h"

// === ВСТРОЕННЫЙ ДИСПЛЕЙ (SH1106) ===
#define MAIN_SDA      17  // Фиксировано на плате!
#define MAIN_SCL      18  // Фиксировано на плате!
#define OLED_VEXT     10  // Питание дисплея
#define OLED_RST      21  // Reset дисплея
SH1106Wire mainDisplay(0x3C, MAIN_SDA, MAIN_SCL);

// === ДОПОЛНИТЕЛЬНЫЙ ДИСПЛЕЙ (SSD1306) ===
// ВЫБЕРИ ОДИН ИЗ ВАРИАНТОВ:
#define EXT_SDA       33  // Вариант 1: GPIO33 (Header J3, пин 7)
#define EXT_SCL       34  // Вариант 1: GPIO34 (Header J3, пин 8)
// #define EXT_SDA       7   // Вариант 2: GPIO7 (Header J2, пин 12)
// #define EXT_SCL       6   // Вариант 2: GPIO6 (Header J2, пин 13)
// #define EXT_SDA       4   // Вариант 3: GPIO4 (Header J2, пин 15)
// #define EXT_SCL       5   // Вариант 3: GPIO5 (Header J2, пин 14)

SSD1306Wire extDisplay(0x3C, EXT_SDA, EXT_SCL);

// === ДЛЯ ВТОРОГО I2C ===
TwoWire secondWire = TwoWire(1); // Создаем второй I2C

// === ПЕРЕМЕННЫЕ ===
bool extDisplayOK = false;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== HELTEC V3.1 - ДВА ДИСПЛЕЯ ===");
  Serial.print("Доп. дисплей на пинах: ");
  Serial.print(EXT_SDA);
  Serial.print(",");
  Serial.println(EXT_SCL);
  
  // 1. Инициализация встроенного дисплея
  initMainDisplay();
  
  // 2. Инициализация дополнительного дисплея
  initExternalDisplay();
  
  // 3. Показываем приветствие
  showWelcome();
}

// ======================= LOOP =======================
void loop() {
  // Обновляем дисплеи каждую секунду
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    updateDisplays();
  }
  delay(100);
}

// ======================= ФУНКЦИИ =======================

void initMainDisplay() {
  Serial.println("Инициализация встроенного дисплея...");
  
  // Включаем питание дисплея
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);
  
  // Сброс дисплея
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  delay(50);
  
  // Инициализация основного I2C (Wire)
  Wire.begin(MAIN_SDA, MAIN_SCL);
  
  // Инициализация дисплея
  mainDisplay.init();
  mainDisplay.flipScreenVertically();
  mainDisplay.clear();
  
  Serial.println("✅ Встроенный дисплей готов");
}

void initExternalDisplay() {
  Serial.println("Инициализация дополнительного дисплея...");
  
  // Вариант A: Используем второй I2C (Wire1)
  // В библиотеке 4.6.1 нужно использовать глобальный Wire1
  // Но мы не можем изменить глобальный Wire1, поэтому используем хак:
  
  // 1. Временно переключаем глобальный Wire на наши пины
  Wire.end(); // Отключаем основной Wire
  
  // 2. Настраиваем Wire на пины дополнительного дисплея
  Wire.begin(EXT_SDA, EXT_SCL);
  Wire.setClock(400000); // Быстрый I2C
  
  // 3. Инициализируем дополнительный дисплей
  extDisplay.init();
  extDisplay.flipScreenVertically();
  extDisplay.clear();
  
  // 4. Проверяем подключение
  Wire.beginTransmission(0x3C);
  byte error = Wire.endTransmission();
  
  // 5. Возвращаем основной Wire обратно на пины 17,18
  Wire.end();
  Wire.begin(MAIN_SDA, MAIN_SCL);
  
  if (error == 0) {
    extDisplayOK = true;
    Serial.println("✅ Дополнительный дисплей найден!");
    
    // Теперь нужно сделать так, чтобы оба дисплея работали
    // Будем использовать второй дисплей только для статичной информации
    // или будем переключать Wire при необходимости
  } else {
    extDisplayOK = false;
    Serial.println("❌ Дополнительный дисплей не найден!");
    Serial.print("   Ошибка I2C: ");
    Serial.println(error);
  }
}

void showWelcome() {
  // Сначала показываем на основном дисплее
  mainDisplay.clear();
  mainDisplay.setFont(ArialMT_Plain_16);
  mainDisplay.drawString(10, 0, "HELTEC V3.1");
  mainDisplay.setFont(ArialMT_Plain_10);
  mainDisplay.drawString(0, 25, "Встроенный SH1106");
  mainDisplay.drawString(0, 40, "Пины: 17,18");
  mainDisplay.display();
  
  delay(1000);
  
  // Затем показываем на дополнительном дисплее (если работает)
  if (extDisplayOK) {
    // Переключаем Wire на пины доп. дисплея
    Wire.end();
    Wire.begin(EXT_SDA, EXT_SCL);
    
    extDisplay.clear();
    extDisplay.setFont(ArialMT_Plain_16);
    extDisplay.drawString(10, 0, "ДОП. ДИСПЛЕЙ");
    extDisplay.drawString(10, 20, "SSD1306");
    extDisplay.setFont(ArialMT_Plain_10);
    extDisplay.drawString(0, 45, String(EXT_SDA) + "," + String(EXT_SCL));
    extDisplay.drawString(0, 55, "ПРИВЕТ!");
    extDisplay.display();
    
    // Возвращаем Wire на основной дисплей
    delay(1000);
    Wire.end();
    Wire.begin(MAIN_SDA, MAIN_SCL);
  }
  
  delay(1000);
}

void updateDisplays() {
  static int counter = 0;
  counter++;
  
  // 1. Обновляем встроенный дисплей
  mainDisplay.clear();
  mainDisplay.setFont(ArialMT_Plain_10);
  mainDisplay.drawString(0, 0, "Встроенный SH1106");
  mainDisplay.drawString(0, 15, "Время: " + String(millis()/1000) + "s");
  mainDisplay.drawString(0, 30, "Счетчик: " + String(counter));
  mainDisplay.drawString(0, 45, "Доп. дисплей:");
  mainDisplay.drawString(80, 45, extDisplayOK ? "OK" : "NO");
  mainDisplay.display();
  
  // 2. Обновляем дополнительный дисплей (если работает)
  if (extDisplayOK) {
    // Временно переключаемся на доп. дисплей
    Wire.end();
    Wire.begin(EXT_SDA, EXT_SCL);
    
    extDisplay.clear();
    extDisplay.setFont(ArialMT_Plain_10);
    extDisplay.drawString(0, 0, "Доп. SSD1306");
    extDisplay.drawString(0, 15, "Пины: " + String(EXT_SDA) + "," + String(EXT_SCL));
    extDisplay.setFont(ArialMT_Plain_24);
    extDisplay.drawString(20, 25, "ПРИВЕТ");
    extDisplay.setFont(ArialMT_Plain_10);
    extDisplay.drawString(0, 55, "Count: " + String(counter));
    extDisplay.display();
    
    // Возвращаемся к основному дисплею
    Wire.end();
    Wire.begin(MAIN_SDA, MAIN_SCL);
  }
}
