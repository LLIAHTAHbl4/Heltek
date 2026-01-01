/*
 * Heltec WiFi Kit 32 V3.1 - ДВА ДИСПЛЕЯ на одной шине I2C
 * Встроенный SH1106 и дополнительный SSD1306
 * Оба на пинах 17,18 но с разными адресами
 */

#include "SH1106Wire.h"
#include "SSD1306Wire.h"
#include "Wire.h"

// === НАСТРОЙКИ ДИСПЛЕЕВ ===
#define SDA_PIN        17  // Единственные рабочие I2C пины
#define SCL_PIN        18  // на твоей плате
#define OLED_VEXT      10  // Питание дисплея
#define OLED_RST       21  // Reset дисплея

// Адреса дисплеев (пробуй менять если не работает)
#define DISPLAY1_ADDR  0x3C  // Встроенный SH1106
#define DISPLAY2_ADDR  0x3D  // Дополнительный SSD1306
// ИЛИ попробуй оба на 0x3C если второй не отвечает

// Объекты дисплеев
SH1106Wire display1(DISPLAY1_ADDR, SDA_PIN, SCL_PIN);  // Встроенный
SSD1306Wire display2(DISPLAY2_ADDR, SDA_PIN, SCL_PIN); // Дополнительный

// === ПЕРЕМЕННЫЕ ===
bool display1OK = false;
bool display2OK = false;
unsigned long lastMillis = 0;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== HELTEC V3.1 - ДВА ДИСПЛЕЯ ===");
  Serial.println("Оба на пинах 17,18, разные адреса");
  
  // Инициализация I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000); // Быстрый I2C для двух дисплеев
  
  // Инициализация дисплеев
  initDisplays();
  
  // Сканирование I2C для проверки
  scanI2C();
  
  // Показываем стартовый экран
  showStartScreens();
}

// ======================= LOOP =======================
void loop() {
  unsigned long currentMillis = millis();
  
  // Обновляем каждые 50мс для плавного отображения времени
  if (currentMillis - lastMillis >= 50) {
    lastMillis = currentMillis;
    
    // Обновляем оба дисплея
    updateDisplay1(); // Встроенный
    updateDisplay2(); // Дополнительный
  }
  
  delay(1);
}

// ======================= ФУНКЦИИ =======================

void initDisplays() {
  Serial.println("\nИНИЦИАЛИЗАЦИЯ ДИСПЛЕЕВ:");
  
  // 1. Включаем питание дисплеев
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);
  
  // 2. Сброс дисплеев
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  delay(50);
  
  // 3. Инициализация встроенного дисплея (SH1106)
  Serial.print("Дисплей 1 (SH1106, адрес 0x");
  Serial.print(DISPLAY1_ADDR, HEX);
  Serial.print(")... ");
  
  display1.init();
  display1.flipScreenVertically();
  display1.clear();
  
  // Проверяем подключение
  Wire.beginTransmission(DISPLAY1_ADDR);
  if (Wire.endTransmission() == 0) {
    display1OK = true;
    Serial.println("OK ✅");
  } else {
    display1OK = false;
    Serial.println("ERROR ❌");
  }
  
  // 4. Инициализация дополнительного дисплея (SSD1306)
  Serial.print("Дисплей 2 (SSD1306, адрес 0x");
  Serial.print(DISPLAY2_ADDR, HEX);
  Serial.print(")... ");
  
  display2.init();
  display2.flipScreenVertically();
  display2.clear();
  
  // Проверяем подключение
  Wire.beginTransmission(DISPLAY2_ADDR);
  if (Wire.endTransmission() == 0) {
    display2OK = true;
    Serial.println("OK ✅");
  } else {
    display2OK = false;
    Serial.println("ERROR ❌");
    
    // Пробуем альтернативный адрес
    Serial.print("Пробую адрес 0x3C... ");
    display2 = SSD1306Wire(0x3C, SDA_PIN, SCL_PIN);
    display2.init();
    display2.flipScreenVertically();
    
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0) {
      display2OK = true;
      DISPLAY2_ADDR = 0x3C;
      Serial.println("OK ✅ (оба на 0x3C)");
    } else {
      Serial.println("ERROR ❌");
    }
  }
}

void scanI2C() {
  Serial.println("\nСКАНИРОВАНИЕ I2C ШИНЫ:");
  
  int found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("  Найден: 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      
      if (addr == DISPLAY1_ADDR) Serial.print(" (Дисплей 1)");
      if (addr == DISPLAY2_ADDR) Serial.print(" (Дисплей 2)");
      
      Serial.println();
      found++;
    }
  }
  
  if (found == 0) {
    Serial.println("  Устройств не найдено!");
  }
  
  Serial.print("Всего устройств: ");
  Serial.println(found);
}

void showStartScreens() {
  Serial.println("\nПОКАЗЫВАЮ СТАРТОВЫЕ ЭКРАНЫ...");
  
  if (display1OK) {
    display1.clear();
    display1.setFont(ArialMT_Plain_16);
    display1.drawString(10, 0, "ДИСПЛЕЙ 1");
    display1.setFont(ArialMT_Plain_10);
    display1.drawString(0, 25, "Встроенный SH1106");
    display1.drawString(0, 40, "Адрес: 0x" + String(DISPLAY1_ADDR, HEX));
    display1.drawString(0, 55, "ПРИВЕТ!");
    display1.display();
  }
  
  if (display2OK) {
    display2.clear();
    display2.setFont(ArialMT_Plain_16);
    display2.drawString(10, 0, "ДИСПЛЕЙ 2");
    display2.setFont(ArialMT_Plain_10);
    display2.drawString(0, 25, "Дополнительный");
    display2.drawString(0, 40, "Адрес: 0x" + String(DISPLAY2_ADDR, HEX));
    display2.drawString(0, 55, "ПРИВЕТ!");
    display2.display();
  }
  
  delay(2000);
}

void updateDisplay1() {
  if (!display1OK) return;
  
  unsigned long now = millis();
  unsigned long seconds = now / 1000;
  unsigned long milliseconds = now % 1000;
  
  // Форматируем время: "секунды:миллисекунды"
  String timeStr = String(seconds) + ":";
  if (milliseconds < 100) timeStr += "0";
  if (milliseconds < 10) timeStr += "0";
  timeStr += String(milliseconds);
  
  display1.clear();
  
  // Верхняя часть - заголовок
  display1.setFont(ArialMT_Plain_10);
  display1.drawString(0, 0, "Дисплей 1 - Встроенный");
  
  // Большие цифры времени
  display1.setFont(ArialMT_Plain_24);
  display1.drawString(5, 15, timeStr);
  
  // Информация внизу
  display1.setFont(ArialMT_Plain_10);
  display1.drawString(0, 45, "Пины: 17,18");
  display1.drawString(0, 55, "Адрес: 0x" + String(DISPLAY1_ADDR, HEX));
  
  display1.display();
}

void updateDisplay2() {
  if (!display2OK) return;
  
  unsigned long now = millis();
  unsigned long seconds = now / 1000;
  unsigned long milliseconds = now % 1000;
  
  // Форматируем время: "секунды:миллисекунды"
  String timeStr = String(seconds) + ":";
  if (milliseconds < 100) timeStr += "0";
  if (milliseconds < 10) timeStr += "0";
  timeStr += String(milliseconds);
  
  display2.clear();
  
  // Верхняя часть - заголовок
  display2.setFont(ArialMT_Plain_10);
  display2.drawString(0, 0, "Дисплей 2 - Дополнит.");
  
  // Большие цифры времени
  display2.setFont(ArialMT_Plain_24);
  display2.drawString(5, 15, timeStr);
  
  // Информация внизу
  display2.setFont(ArialMT_Plain_10);
  display2.drawString(0, 45, "На одной шине I2C");
  display2.drawString(0, 55, "Адрес: 0x" + String(DISPLAY2_ADDR, HEX));
  
  display2.display();
}
