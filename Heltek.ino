/*
 * Тест батареи с Bluetooth
 * ESP32 Bluetooth Serial
 */

#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth не включен в настройках!
#endif

BluetoothSerial SerialBT;
const char* deviceName = "Heltec_Battery_Test";

void setup() {
  // Инициализация Serial
  Serial.begin(115200);
  Serial.println("\n=== Тест батареи с Bluetooth ===");
  
  // Инициализация Bluetooth
  SerialBT.begin(deviceName);
  Serial.print("Bluetooth устройство: ");
  Serial.println(deviceName);
  Serial.println("Подключитесь для просмотра данных");
  
  SerialBT.println("=== Heltec V3.1 Battery Test ===");
  SerialBT.println("Батарея: ожидается ~3.87V");
  SerialBT.println("Поиск пина с ~1.94V");
  
  // Настройка АЦП
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  delay(1000);
}

void loop() {
  static unsigned long lastScan = 0;
  
  // Сканируем каждые 3 секунды
  if (millis() - lastScan >= 3000) {
    lastScan = millis();
    
    scanBatteryPins();
    
    // Отправляем в Bluetooth
    if (SerialBT.connected()) {
      SerialBT.print("\nСледующее сканирование через 3с... [");
      SerialBT.print(millis() / 1000);
      SerialBT.println("с]");
    }
  }
  
  delay(100);
}

void scanBatteryPins() {
  // Основные пины для проверки
  int testPins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 19, 20, 21};
  int pinCount = sizeof(testPins) / sizeof(testPins[0]);
  
  bool foundBattery = false;
  
  Serial.println("\n--- Сканирование пинов ---");
  
  for (int i = 0; i < pinCount; i++) {
    int pin = testPins[i];
    checkPinForBattery(pin, foundBattery);
  }
  
  if (!foundBattery) {
    Serial.println("Пин батареи не найден (ожидаем ~1.94V)");
    SerialBT.println("Пин батареи не найден!");
  }
}

void checkPinForBattery(int pin, bool &found) {
  // Усредняем 5 измерений
  long total = 0;
  for (int i = 0; i < 5; i++) {
    total += analogRead(pin);
    delay(2);
  }
  
  int adcValue = total / 5;
  float voltage = adcValue * (3.3 / 4095.0);
  
  // Ищем напряжение около 1.94V (3.87V / 2)
  if (voltage > 1.8 && voltage < 2.1) {
    found = true;
    
    String message = ">>> БАТАРЕЯ НАЙДЕНА: GPIO";
    message += pin;
    message += " = ";
    message += String(voltage, 3);
    message += "V (ADC: ";
    message += adcValue;
    message += ")";
    message += " -> Батарея: ";
    message += String(voltage * 2.0, 2);
    message += "V";
    
    Serial.println(message);
    
    if (SerialBT.connected()) {
      SerialBT.println(message);
    }
  }
}
