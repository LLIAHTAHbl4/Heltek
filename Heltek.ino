/*
 * Тест пина батареи с Serial
 * Простая версия без Bluetooth
 */

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Battery Pin Test ===");
  Serial.println("Подключи USB для Serial Monitor");
  Serial.println("Ожидаем пин с ~1.94V (3.87V / 2)");
  Serial.println();
  
  analogReadResolution(12);
  delay(1000);
}

void loop() {
  static unsigned long lastScan = 0;
  
  if (millis() - lastScan >= 2000) {
    lastScan = millis();
    
    Serial.print("\n[");
    Serial.print(millis() / 1000);
    Serial.println("s] Сканирование...");
    
    // Тестируем основные пины
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
    
    Serial.println("Следующее сканирование через 2с");
  }
  
  delay(100);
}

void testPin(int pin) {
  // Усредняем 3 измерения
  long total = 0;
  for (int i = 0; i < 3; i++) {
    total += analogRead(pin);
    delay(1);
  }
  
  int adcValue = total / 3;
  float voltage = adcValue * (3.3 / 4095.0);
  
  // Ищем напряжение около 1.94V
  if (voltage > 1.8 && voltage < 2.1) {
    Serial.print(">>> ВОЗМОЖНО БАТАРЕЯ: GPIO");
    Serial.print(pin);
    Serial.print(" = ");
    Serial.print(voltage, 3);
    Serial.print("V -> Батарея: ");
    Serial.print(voltage * 2.0, 2);
    Serial.println("V");
  }
  // Показываем все пины с напряжением > 0.5V
  else if (voltage > 0.5) {
    Serial.print("GPIO");
    if (pin < 10) Serial.print("0");
    Serial.print(pin);
    Serial.print(": ");
    Serial.print(voltage, 2);
    Serial.println("V");
  }
}
