#include "SH1106Wire.h"

// Константы дисплея
#define OLED_VEXT 10
#define OLED_RST 21
#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_ADDR 0x3C

SH1106Wire display(OLED_ADDR, OLED_SDA, OLED_SCL);

void setup() {
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
  
  // Вывод текста
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.drawString(20, 20, "ПРИВЕТ");
  display.display();
}

void loop() {
  // Ничего не делаем
}
