/*
 * Тест пина батареи с Bluetooth
 * Питание: батарея 3.87V
 * Данные: Bluetooth Serial
 */

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;
String deviceName = "Heltec_Battery_Test";

void setup() {
  Serial.begin(115200);
  SerialBT.begin(deviceName);
  
  Serial.println("\n=== Bluetooth Battery Test ===");
  Serial.println("Device: " + deviceName);
  
  SerialBT.println("=== Heltec V3.1 Battery Test ===");
  SerialBT.println("Connect via Bluetooth");
  SerialBT.println("Battery: ~3.87V expected");
  
  analogReadResolution(12);
}

void loop() {
  static unsigned long lastScan = 0;
  
  if (millis() - lastScan >= 2000) {
    lastScan = millis();
    
    scanAllPins();
    
    SerialBT.println("Next scan in 2s...");
    SerialBT.println();
  }
  
  delay(100);
}

void scanAllPins() {
  int pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 20, 21, 33, 34, 35, 36, 39};
  
  for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    checkPin(pins[i]);
  }
}

void checkPin(int pin) {
  int total = 0;
  for (int j = 0; j < 5; j++) {
    total += analogRead(pin);
    delay(2);
  }
  
  int adcValue = total / 5;
  float voltage = adcValue * (3.3 / 4095.0);
  
  if (voltage > 1.8 && voltage < 2.1) {
    String msg = "FOUND: GPIO";
    msg += pin;
    msg += " = ";
    msg += String(voltage, 3);
    msg += "V (Battery: ";
    msg += String(voltage * 2.0, 2);
    msg += "V)";
    
    Serial.println(msg);
    SerialBT.println(msg);
  }
}
