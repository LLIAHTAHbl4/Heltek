/*
 * Тест батареи с Bluetooth Serial
 * Питание: только батарея 3.87V
 * Монитор порта: Bluetooth
 */

#include "BluetoothSerial.h"

// Проверяем поддержку Bluetooth
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

BluetoothSerial SerialBT;
String deviceName = "Heltec_Battery_Test";

void setup() {
  // Запускаем оба Serial
  Serial.begin(115200);
  SerialBT.begin(deviceName);
  
  delay(1000);
  
  Serial.println("\n=== ТЕСТ БАТАРЕИ по Bluetooth ===");
  Serial.println("Имя устройства: " + deviceName);
  Serial.println("Подключитесь по Bluetooth для данных");
  
  SerialBT.println("\n=== ТЕСТ БАТАРЕИ ===");
  SerialBT.println("Устройство: Heltec V3.1");
  SerialBT.println("Батарея: 3.87V (под нагрузкой)");
  SerialBT.println("Ожидаем пин с ~1.94V");
  
  analogReadResolution(12);
}

void loop() {
  static unsigned long lastScan = 0;
  
  if (millis() - lastScan >= 3000) {
    lastScan = millis();
    
    String message = "\n--- Сканирование " + String(millis()/1000) + "с ---";
    Serial.println(message);
    SerialBT.println(message);
    
    // Тестируем пины
    testPin(1);
    testPin(2);
    testPin(3);
    testPin(4);
    testPin(5);
    testPin(6);
    testPin(7);
    testPin(8);
    testPin(9);
    testPin(10);
    testPin(11);
    testPin(12);
    testPin(13);
    testPin(14);
    testPin(15);
    testPin(16);
    testPin(17);
    testPin(18);
    testPin(19);
    testPin(20);
    testPin(21);
    testPin(33);
    testPin(34);
    testPin(35);
    testPin(36);
    testPin(39);
    
    SerialBT.println("Следующее сканирование через 3с...");
  }
  
  delay(100);
}

void testPin(int pin) {
  // 3 измерения для усреднения
  long total = 0;
  for (int i = 0; i < 3; i++) {
    total += analogRead(pin);
    delay(1);
  }
  
  int adcValue = total / 3;
  float voltage = adcValue * (3.3 / 4095.0);
  
  // Отправляем только интересные пины
  if (voltage > 1.5 && voltage < 2.5) {
    String msg = ">>> ВОЗМОЖНО БАТАРЕЯ: GPIO";
    msg += pin;
    msg += " = ";
    msg += String(voltage, 3);
    msg += "V -> Батарея: ";
    msg += String(voltage * 2.0, 2);
    msg += "V";
    
    Serial.println(msg);
    SerialBT.println(msg);
  } else if (voltage > 0.5) {
    // Остальные пины только в Serial (не засоряем Bluetooth)
    Serial.print("GPIO");
    if (pin < 10) Serial.print("0");
    Serial.print(pin);
    Serial.print(": ");
    Serial.print(voltage, 2);
    Serial.println("V");
  }
}
