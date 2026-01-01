/*
 * Heltec WiFi Kit 32 V3.1 (ESP32-S3)
 * Простейший код для отображения "ПРИВЕТ"
 */

#include "SH1106Wire.h"

// === КОНСТАНТЫ ДИСПЛЕЯ ===
#define OLED_VEXT     10    // Питание дисплея (LOW = ВКЛ)
#define OLED_RST      21    // Reset дисплея
#define OLED_SDA      17    // SDA дисплея (НЕ МЕНЯТЬ!)
#define OLED_SCL      18    // SCL дисплея (НЕ МЕНЯТЬ!)
#define OLED_ADDR     0x3C  // Адрес I2C дисплея

// Объект дисплея
SH1106Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);

// ======================= SETUP =======================
void setup() {
  // Инициализация Serial для отладки
  Serial.begin(115200);
  Serial.println("\n=== Heltec WiFi Kit 32 V3.1 ===");
  Serial.println("Простой тест дисплея");
  
  // Инициализация дисплея
  initDisplay();
  
  // Вывод текста
  displayText();
}

// ======================= LOOP =======================
void loop() {
  // Ничего не делаем в цикле
  delay(1000);
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

// Вывод текста на дисплей
void displayText() {
  display.clear();
  
  // Большой текст "ПРИВЕТ"
  display.setFont(ArialMT_Plain_24);
  display.drawString(20, 20, "ПРИВЕТ");
  
  // Мелкий текст внизу
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 50, "Heltec V3.1");
  
  // Отображаем на дисплее
  display.display();
  
  Serial.println("Текст отображен на дисплее");
}
