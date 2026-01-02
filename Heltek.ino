/*
 * Heltec V3.1 - Мини тест батареи
 * Измерение VBAT через GPIO1 и вывод на OLED
 */

#include "Wire.h"
#include "SH1106Wire.h"

// === OLED ===
#define SDA_PIN   17
#define SCL_PIN   18
#define OLED_VEXT 10
#define OLED_RST  21
#define DISPLAY_ADDR 0x3C

SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);

// === BATTERY ===
#define BAT_ADC 1                 // GPIO1
#define BATTERY_DIVIDER 4.9f      // Делитель резисторов платы
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2

// === Фильтр и чтение VBAT ===
float readBatteryFiltered() {
    const int samples = 10;
    float sum = 0;
    for(int i=0; i<samples; i++){
        sum += analogRead(BAT_ADC);
        delay(2);
    }
    float adcVal = sum / samples;
    float voltage = (adcVal / 4095.0f) * 3.3f;  // Напряжение на ADC
    voltage *= BATTERY_DIVIDER;                 // Учёт делителя
    return voltage;
}

// === Конвертация в % ===
int batteryPercent(float voltage){
    int percent = (voltage - BATTERY_MIN_VOLTAGE) * 100.0 / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE);
    if(percent > 100) percent = 100;
    if(percent < 0) percent = 0;
    return percent;
}

// === Инициализация дисплея ===
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
    display.drawString(0,0,"VBAT Test");
    display.display();
    delay(1000);
}

void setup() {
    analogReadResolution(12);       // 12 бит
    analogSetAttenuation(ADC_11db); // Диапазон до ~3.9V на ADC
    setupDisplay();
}

void loop() {
    display.clear();
    float voltage = readBatteryFiltered();
    int percent = batteryPercent(voltage);

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, String(voltage,2) + " V");

    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "Charge: " + String(percent) + " %");

    display.display();
    delay(1000); // Обновление каждую секунду
}
