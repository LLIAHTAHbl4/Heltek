/*
 * Heltec WiFi Kit 32 V3.1 - ДИАГНОСТИКА
 * Три дисплея SSD1306 на разных пинах
 */

#include "SH1106Wire.h"  // Для основного дисплея
#include "SSD1306Wire.h" // Для дополнительных дисплеев
#include "Wire.h"

// === ОСНОВНОЙ ДИСПЛЕЙ (встроенный SH1106) ===
#define MAIN_DISPLAY_SDA  17
#define MAIN_DISPLAY_SCL  18
#define DISPLAY_VEXT      10
#define DISPLAY_RST       21
SH1106Wire mainDisplay(0x3C, MAIN_DISPLAY_SDA, MAIN_DISPLAY_SCL);

// === ДИСПЛЕЙ 1: На пинах 4 и 5 (тестируем эти пины) ===
#define DISPLAY1_SDA      4
#define DISPLAY1_SCL      5
SSD1306Wire display1(0x3C, DISPLAY1_SDA, DISPLAY1_SCL);

// === ДИСПЛЕЙ 2: На пинах 33 и 34 (альтернативные) ===
#define DISPLAY2_SDA      33
#define DISPLAY2_SCL      34
SSD1306Wire display2(0x3C, DISPLAY2_SDA, DISPLAY2_SCL);

// === ПЕРЕМЕННЫЕ ===
bool mainDisplayOK = false;
bool display1OK = false;
bool display2OK = false;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== HELTEC V3.1 - ДИАГНОСТИКА ===");
  Serial.println("Тестируем 3 дисплея на разных пинах");
  
  // Даем время на инициализацию
  delay(1000);
  
  // 1. Основной дисплей (встроенный)
  testMainDisplay();
  
  // 2. Дисплей на пинах 4 и 5
  testDisplay1();
  
  // 3. Дисплей на пинах 33 и 34
  testDisplay2();
  
  // 4. Вывод результатов
  showResults();
  
  // 5. Сканирование I2C
  scanAllI2C();
}

// ======================= LOOP =======================
void loop() {
  // Обновляем дисплеи каждые 2 секунды
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 2000) {
    lastUpdate = millis();
    updateDisplays();
  }
  delay(100);
}

// ======================= ФУНКЦИИ =======================

void testMainDisplay() {
  Serial.println("\n[1] ТЕСТ ОСНОВНОГО ДИСПЛЕЯ (пины 17,18)");
  
  // Включаем питание дисплея
  pinMode(DISPLAY_VEXT, OUTPUT);
  digitalWrite(DISPLAY_VEXT, LOW);
  delay(100);
  
  // Сброс дисплея
  pinMode(DISPLAY_RST, OUTPUT);
  digitalWrite(DISPLAY_RST, LOW);
  delay(50);
  digitalWrite(DISPLAY_RST, HIGH);
  delay(50);
  
  // Инициализация I2C для основного дисплея
  Wire.begin(MAIN_DISPLAY_SDA, MAIN_DISPLAY_SCL);
  
  // Инициализация дисплея
  mainDisplay.init();
  mainDisplay.flipScreenVertically();
  mainDisplay.clear();
  
  // Пробуем что-то нарисовать
  mainDisplay.setFont(ArialMT_Plain_16);
  mainDisplay.drawString(10, 0, "MAIN");
  mainDisplay.drawString(10, 20, "17,18");
  mainDisplay.display();
  
  delay(500);
  
  // Проверяем, отобразилось ли
  mainDisplayOK = true;
  Serial.println("✅ Основной дисплей: ИНИЦИАЛИЗИРОВАН");
  Serial.print("   SDA=GPIO"); Serial.println(MAIN_DISPLAY_SDA);
  Serial.print("   SCL=GPIO"); Serial.println(MAIN_DISPLAY_SCL);
}

void testDisplay1() {
  Serial.println("\n[2] ТЕСТ ДИСПЛЕЯ 1 (пины 4,5)");
  
  // Создаем отдельный I2C для этого дисплея
  TwoWire I2C_Disp1 = TwoWire(0);
  I2C_Disp1.begin(DISPLAY1_SDA, DISPLAY1_SCL, 400000);
  
  // Инициализация дисплея
  display1.init(&I2C_Disp1);
  display1.flipScreenVertically();
  display1.clear();
  
  // Пробуем что-то нарисовать
  display1.setFont(ArialMT_Plain_16);
  display1.drawString(10, 0, "DISPLAY1");
  display1.drawString(10, 20, "4,5");
  display1.drawString(10, 40, "ПРИВЕТ");
  display1.display();
  
  delay(500);
  
  // Проверяем подключение
  I2C_Disp1.beginTransmission(0x3C);
  byte error = I2C_Disp1.endTransmission();
  
  if (error == 0) {
    display1OK = true;
    Serial.println("✅ Дисплей 1: РАБОТАЕТ на пинах 4,5");
  } else {
    display1OK = false;
    Serial.println("❌ Дисплей 1: НЕ РАБОТАЕТ на пинах 4,5");
    Serial.print("   Ошибка I2C: "); Serial.println(error);
  }
}

void testDisplay2() {
  Serial.println("\n[3] ТЕСТ ДИСПЛЕЯ 2 (пины 33,34)");
  
  // Создаем отдельный I2C для этого дисплея
  TwoWire I2C_Disp2 = TwoWire(1);
  I2C_Disp2.begin(DISPLAY2_SDA, DISPLAY2_SCL, 400000);
  
  // Инициализация дисплея
  display2.init(&I2C_Disp2);
  display2.flipScreenVertically();
  display2.clear();
  
  // Пробуем что-то нарисовать
  display2.setFont(ArialMT_Plain_16);
  display2.drawString(10, 0, "DISPLAY2");
  display2.drawString(10, 20, "33,34");
  display2.drawString(10, 40, "ПРИВЕТ");
  display2.display();
  
  delay(500);
  
  // Проверяем подключение
  I2C_Disp2.beginTransmission(0x3C);
  byte error = I2C_Disp2.endTransmission();
  
  if (error == 0) {
    display2OK = true;
    Serial.println("✅ Дисплей 2: РАБОТАЕТ на пинах 33,34");
  } else {
    display2OK = false;
    Serial.println("❌ Дисплей 2: НЕ РАБОТАЕТ на пинах 33,34");
    Serial.print("   Ошибка I2C: "); Serial.println(error);
  }
}

void showResults() {
  Serial.println("\n=== РЕЗУЛЬТАТЫ ТЕСТА ===");
  Serial.print("Основной дисплей (17,18): ");
  Serial.println(mainDisplayOK ? "РАБОТАЕТ" : "НЕ РАБОТАЕТ");
  
  Serial.print("Дисплей 1 (4,5): ");
  Serial.println(display1OK ? "РАБОТАЕТ" : "НЕ РАБОТАЕТ");
  
  Serial.print("Дисплей 2 (33,34): ");
  Serial.println(display2OK ? "РАБОТАЕТ" : "НЕ РАБОТАЕТ");
  
  // Вывод на основной дисплей
  mainDisplay.clear();
  mainDisplay.setFont(ArialMT_Plain_10);
  mainDisplay.drawString(0, 0, "ДИАГНОСТИКА:");
  
  mainDisplay.drawString(0, 15, "Дисплей1 (4,5):");
  mainDisplay.drawString(80, 15, display1OK ? "OK" : "ERROR");
  
  mainDisplay.drawString(0, 30, "Дисплей2 (33,34):");
  mainDisplay.drawString(80, 30, display2OK ? "OK" : "ERROR");
  
  mainDisplay.drawString(0, 45, "---");
  mainDisplay.drawString(0, 55, "Проверьте осциллограф");
  
  mainDisplay.display();
}

void scanAllI2C() {
  Serial.println("\n=== СКАНИРОВАНИЕ ВСЕХ I2C ШИН ===");
  
  // Шина 1: пины 4,5
  Serial.println("Шина 1 (пины 4,5):");
  TwoWire wire1 = TwoWire(0);
  wire1.begin(DISPLAY1_SDA, DISPLAY1_SCL);
  scanI2CBus(wire1, "4,5");
  
  // Шина 2: пины 33,34
  Serial.println("\nШина 2 (пины 33,34):");
  TwoWire wire2 = TwoWire(1);
  wire2.begin(DISPLAY2_SDA, DISPLAY2_SCL);
  scanI2CBus(wire2, "33,34");
  
  // Шина 3: пины 17,18 (основная)
  Serial.println("\nШина 3 (пины 17,18):");
  TwoWire wire3 = TwoWire(2);
  wire3.begin(MAIN_DISPLAY_SDA, MAIN_DISPLAY_SCL);
  scanI2CBus(wire3, "17,18");
}

void scanI2CBus(TwoWire &wire, String busName) {
  Serial.print("Сканирую шину ");
  Serial.print(busName);
  Serial.println("...");
  
  int found = 0;
  for(byte addr = 1; addr < 127; addr++) {
    wire.beginTransmission(addr);
    byte error = wire.endTransmission();
    
    if (error == 0) {
      Serial.print("  Найден: 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }
  
  if (found == 0) {
    Serial.println("  Устройств не найдено!");
  }
}

void updateDisplays() {
  // Обновляем время на всех рабочих дисплеях
  
  if (mainDisplayOK) {
    mainDisplay.setFont(ArialMT_Plain_10);
    mainDisplay.drawString(90, 0, String(millis()/1000) + "s");
    mainDisplay.display();
  }
  
  if (display1OK) {
    display1.setFont(ArialMT_Plain_10);
    display1.drawString(0, 55, "Time: " + String(millis()/1000));
    display1.display();
  }
  
  if (display2OK) {
    display2.setFont(ArialMT_Plain_10);
    display2.drawString(0, 55, "Time: " + String(millis()/1000));
    display2.display();
  }
}
