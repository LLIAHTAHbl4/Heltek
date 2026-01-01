/*
 * Heltec WiFi Kit 32 V3.1
 * Два дисплея: встроенный SH1106 + дополнительный SSD1306
 */

#include "SH1106Wire.h"  // Для встроенного дисплея
#include "SSD1306Wire.h" // Для дополнительного дисплея

// === ВСТРОЕННЫЙ ДИСПЛЕЙ (SH1106) ===
#define MAIN_SDA      17  // Фиксировано на плате!
#define MAIN_SCL      18  // Фиксировано на плате!
#define OLED_VEXT     10  // Питание дисплея
#define OLED_RST      21  // Reset дисплея
SH1106Wire mainDisplay(0x3C, MAIN_SDA, MAIN_SCL);

// === ДОПОЛНИТЕЛЬНЫЙ ДИСПЛЕЙ (SSD1306) ===
// ВЫБЕРИ ОДИН ИЗ ВАРИАНТОВ:
// #define EXT_SDA       7   // Вариант 1: GPIO7
// #define EXT_SCL       6   // Вариант 1: GPIO6
#define EXT_SDA       33  // Вариант 2: GPIO33
#define EXT_SCL       34  // Вариант 2: GPIO34
// #define EXT_SDA       4   // Вариант 3: GPIO4
// #define EXT_SCL       5   // Вариант 3: GPIO5

SSD1306Wire extDisplay(0x3C, EXT_SDA, EXT_SCL); // Адрес тот же 0x3C!

// === ПЕРЕМЕННЫЕ ===
bool extDisplayOK = false;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== HELTEC V3.1 - ДВА ДИСПЛЕЯ ===");
  
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
  
  // Инициализация I2C
  Wire.begin(MAIN_SDA, MAIN_SCL);
  
  // Инициализация дисплея
  mainDisplay.init();
  mainDisplay.flipScreenVertically();
  mainDisplay.clear();
  
  Serial.println("✅ Встроенный дисплей готов");
}

void initExternalDisplay() {
  Serial.print("Инициализация дополнительного дисплея на пинах ");
  Serial.print(EXT_SDA);
  Serial.print(",");
  Serial.println(EXT_SCL);
  
  // Создаем отдельную I2C шину для дополнительного дисплея
  TwoWire extWire = TwoWire(1); // Используем Wire1
  extWire.begin(EXT_SDA, EXT_SCL, 400000); // Быстрый I2C
  
  // Инициализация дисплея
  extDisplay.init(&extWire); // Передаем наш Wire1
  extDisplay.flipScreenVertically();
  extDisplay.clear();
  
  // Проверяем подключение
  extWire.beginTransmission(0x3C);
  byte error = extWire.endTransmission();
  
  if (error == 0) {
    extDisplayOK = true;
    Serial.println("✅ Дополнительный дисплей найден!");
  } else {
    extDisplayOK = false;
    Serial.println("❌ Дополнительный дисплей не найден!");
    Serial.print("   Ошибка I2C: ");
    Serial.println(error);
    Serial.println("   Попробуйте другие пины!");
  }
}

void showWelcome() {
  // На встроенном дисплее
  mainDisplay.clear();
  mainDisplay.setFont(ArialMT_Plain_16);
  mainDisplay.drawString(10, 0, "HELTEC V3.1");
  mainDisplay.setFont(ArialMT_Plain_10);
  mainDisplay.drawString(0, 25, "Встроенный дисплей");
  mainDisplay.drawString(0, 40, "SH1106");
  mainDisplay.drawString(0, 55, "Пины: 17,18");
  mainDisplay.display();
  
  // На дополнительном дисплее (если работает)
  if (extDisplayOK) {
    extDisplay.clear();
    extDisplay.setFont(ArialMT_Plain_16);
    extDisplay.drawString(10, 0, "ДОП. ДИСПЛЕЙ");
    extDisplay.drawString(10, 20, "SSD1306");
    extDisplay.setFont(ArialMT_Plain_10);
    extDisplay.drawString(0, 45, "Пины:");
    extDisplay.drawString(40, 45, String(EXT_SDA) + "," + String(EXT_SCL));
    extDisplay.drawString(0, 55, "ПРИВЕТ!");
    extDisplay.display();
  }
  
  delay(2000);
}

void updateDisplays() {
  static int counter = 0;
  counter++;
  
  // Обновляем встроенный дисплей
  mainDisplay.clear();
  mainDisplay.setFont(ArialMT_Plain_10);
  mainDisplay.drawString(0, 0, "Встроенный SH1106");
  mainDisplay.drawString(0, 15, "Время: " + String(millis()/1000) + "s");
  mainDisplay.drawString(0, 30, "Счетчик: " + String(counter));
  mainDisplay.drawString(0, 45, "Доп. дисплей:");
  mainDisplay.drawString(70, 45, extDisplayOK ? "OK" : "ERROR");
  mainDisplay.display();
  
  // Обновляем дополнительный дисплей
  if (extDisplayOK) {
    extDisplay.clear();
    extDisplay.setFont(ArialMT_Plain_10);
    extDisplay.drawString(0, 0, "Дополнительный SSD1306");
    extDisplay.drawString(0, 15, "Пины: " + String(EXT_SDA) + "," + String(EXT_SCL));
    extDisplay.setFont(ArialMT_Plain_24);
    extDisplay.drawString(20, 25, "ПРИВЕТ");
    extDisplay.setFont(ArialMT_Plain_10);
    extDisplay.drawString(0, 55, "Counter: " + String(counter));
    extDisplay.display();
  }
}
