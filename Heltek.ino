/*
 * Heltec V3.1 - Стабильное измерение VBAT
 * Диапазон батареи: 3.0V - 4.2V
 * Используется GPIO1 (ADC1_CH0) и делитель 4.9x
 * Плавное отображение на OLED через EMA фильтр
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

float batteryVoltage = 0.0;

// === Чтение и плавное сглаживание ===
float readBatterySmooth() {
    const int samples = 50;   // больше измерений для стабильности
    float sum = 0;
    for(int i=0; i<samples; i++){
        sum += analogRead(BAT_ADC);
        delay(2); // короткая пауза для стабилизации
    }
    float adcVal = sum / samples;
    float rawV = (adcVal / 4095.0f) * 3.3f * BATTERY_DIVIDER;

    // Экспоненциальное сглаживание
    batteryVoltage = 0.05 * rawV + 0.95 * batteryVoltage;
    return batteryVoltage;
}

// === Конвертация в % заряда ===
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
    analogReadResolution(12);        // 12 бит
    analogSetAttenuation(ADC_11db);  // диапазон до ~3.9V
    setupDisplay();

    // Инициализация фильтра
    batteryVoltage = readBatterySmooth();
}

void loop() {
    display.clear();

    float voltage = readBatterySmooth();
    int percent = batteryPercent(voltage);

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 10, String(voltage,2) + " V");

    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "Charge: " + String(percent) + " %");

    display.display();
    delay(1000); // обновление каждую секунду
}
