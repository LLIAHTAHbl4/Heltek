/*
 * Heltec WiFi Kit 32 V3.1
 * VBAT ADC Finder
 * Проверка на каком ADC пине появляется напряжение батареи
 * Напряжение отображается на OLED дисплее
 */

#include "Wire.h"
#include "SH1106Wire.h"

// === ПИНЫ OLED ===
#define SDA_PIN   17
#define SCL_PIN   18
#define OLED_VEXT 10
#define OLED_RST  21
#define DISPLAY_ADDR 0x3C

SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);

// === КОЭФФИЦИЕНТ ДЕЛИТЕЛЯ ===
#define VBAT_DIVIDER 4.9f  // Делитель резисторов на плате 390K/100K

// === СПИСОК ADC ПИНОВ, КОТОРЫЕ МОЖНО ПРОВЕРИТЬ ===
const int adcPins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                       10,11,12,13,14,15,16,17,18,19,
                       20,21,22,23,24,25,26,27,28,29,
                       30,31,32,33,34,35,36,37,38,39};

// === ФУНКЦИИ ===
float readADC(int pin) {
  analogReadResolution(12);      // 12 бит (0-4095)
  analogSetAttenuation(ADC_11db); // Диапазон до ~3.9V на ADC
  int val = analogRead(pin);
  float voltage = (val / 4095.0f) * 3.3f; // Напряжение на ADC
  return voltage;
}

void setupDisplay() {
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, HIGH); // включаем дисплей
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);
  delay(50);

  Wire.begin(SDA_PIN, SCL_PIN);
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,0,"VBAT ADC Finder");
  display.display();
  delay(1000);
}

void setup() {
  setupDisplay();
}

void loop() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0,0,"Scanning ADC pins...");

  int y = 15;
  for(int i=0; i < sizeof(adcPins)/sizeof(adcPins[0]); i++){
    int pin = adcPins[i];
    float adcV = readADC(pin);
    float vbat = adcV * VBAT_DIVIDER;

    // Печатаем только если напряжение >0.5V (чтобы отфильтровать "пустые" пины)
    if(adcV > 0.5){
      char buf[32];
      snprintf(buf, sizeof(buf), "GPIO%d: %.2fV -> %.2fV", pin, adcV, vbat);
      display.drawString(0,y,buf);
      y += 10;
      if(y > 50) break; // ограничение на экран
    }
  }

  display.display();
  delay(1000);
}
