/*
 * Heltec WiFi Kit 32 V3.1 (ESP32-S3)
 * AS5600 + SHT31 + OLED + Battery
 * BLE UART (вместо BluetoothSerial)
 */

#include <Wire.h>
#include "SH1106Wire.h"
#include "Adafruit_SHT31.h"

// ===== BLE =====
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ================= ПИНЫ =================
#define SDA_PIN   17
#define SCL_PIN   18
#define OLED_VEXT 10
#define OLED_RST  21
#define BAT_ADC   1        // GPIO1 = VBAT Read (ADC1_CH0)

// ================= I2C =================
#define DISPLAY_ADDR 0x3C
#define AS5600_ADDR  0x36
#define SHT31_ADDR   0x44

// ================= БАТАРЕЯ =================
#define VOLTAGE_DIVIDER_RATIO 2.0
#define ADC_SAMPLES 20

// ================= BLE UUID (Nordic UART) =================
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHAR_TX_UUID        "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// ================= ОБЪЕКТЫ =================
SH1106Wire display(DISPLAY_ADDR, SDA_PIN, SCL_PIN);
Adafruit_SHT31 sht31;

BLECharacteristic *bleTx;

// ================= ПЕРЕМЕННЫЕ =================
bool as5600Connected = false;
bool sht31Connected  = false;

float angleDeg       = 0.0;
int   magnetRaw      = 0;
int   magnetPercent  = 0;

float temperature    = 0.0;
float humidity       = 0.0;

float batteryVoltage = 0.0;
int   batteryPercent = 0;

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // --- ADC ---
  analogReadResolution(12);
  analogSetAttenuation(ADC_6db);

  // --- OLED ---
  pinMode(OLED_VEXT, OUTPUT);
  digitalWrite(OLED_VEXT, LOW);
  delay(100);

  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(50);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);

  display.init();
  display.flipScreenVertically();
  display.clear();

  // --- AS5600 ---
  Wire.beginTransmission(AS5600_ADDR);
  as5600Connected = (Wire.endTransmission() == 0);

  // --- SHT31 ---
  sht31Connected = sht31.begin(SHT31_ADDR);

  // --- BLE ---
  BLEDevice::init("Heltec_V3_BLE");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(SERVICE_UUID);

  bleTx = service->createCharacteristic(
    CHAR_TX_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );

  bleTx->addDescriptor(new BLE2902());
  service->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->start();

  Serial.println("BLE UART запущен");
}

// ================= LOOP =================
void loop() {
  readAS5600();
  readSHT31();
  readBattery();
  updateDisplay();
  sendBLE();
  delay(1000);
}

// ================= AS5600 =================
void readAS5600() {
  if (!as5600Connected) return;

  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, 2);

  uint16_t raw = (Wire.read() << 8) | Wire.read();
  angleDeg = raw * 360.0 / 4096.0;

  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x1B);
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, 2);

  magnetRaw = (Wire.read() << 8) | Wire.read();
  magnetPercent = constrain(map(magnetRaw, 0, 2000, 0, 100), 0, 100);
}

// ================= SHT31 =================
void readSHT31() {
  if (!sht31Connected) return;
  temperature = sht31.readTemperature();
  humidity    = sht31.readHumidity();
}

// ================= БАТАРЕЯ =================
void readBattery() {
  uint32_t sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(BAT_ADC);
    delay(3);
  }

  float adc = sum / (float)ADC_SAMPLES;
  float v   = adc * (3.3 / 4095.0);
  batteryVoltage = v * VOLTAGE_DIVIDER_RATIO;
  batteryPercent = batteryFromVoltage(batteryVoltage);
}

int batteryFromVoltage(float v) {
  if (v >= 4.20) return 100;
  if (v >= 4.00) return map(v * 100, 400, 420, 80, 100);
  if (v >= 3.85) return map(v * 100, 385, 400, 60, 80);
  if (v >= 3.75) return map(v * 100, 375, 385, 40, 60);
  if (v >= 3.60) return map(v * 100, 360, 375, 20, 40);
  if (v >= 3.30) return map(v * 100, 330, 360, 5, 20);
  if (v >= 3.00) return map(v * 100, 300, 330, 0, 5);
  return 0;
}

// ================= OLED =================
void updateDisplay() {
  display.clear();
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0,  "ANG: " + String(angleDeg, 1));
  display.drawString(0, 12, "MAG: " + String(magnetPercent) + "%");
  display.drawString(0, 24, "T: " + String(temperature, 1) + "C");
  display.drawString(0, 36, "H: " + String(humidity, 1) + "%");
  display.drawString(0, 48, "BAT: " + String(batteryVoltage, 2) + "V " + String(batteryPercent) + "%");

  display.display();
}

// ================= BLE LOG =================
void sendBLE() {
  String msg =
    "Vbat=" + String(batteryVoltage, 3) + "V " +
    String(batteryPercent) + "% | " +
    "Angle=" + String(angleDeg, 1) + " | " +
    "T=" + String(temperature, 1) + "C";

  Serial.println(msg);
  bleTx->setValue(msg.c_str());
  bleTx->notify();
}
