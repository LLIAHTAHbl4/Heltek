/*
 * ТЕСТ ПИНА БАТАРЕИ Heltec V3.1
 * Ищем пин с напряжением ~1.94V (3.87V / 2)
 */

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ТЕСТ ПИНА БАТАРЕИ ===");
  Serial.println("Ожидаем ~1.94V (3.87V / 2)");
  
  analogReadResolution(12);
}

void loop() {
  Serial.println("\n--- Сканирование пинов ---");
  
  int pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 33, 34, 35, 36, 39};
  
  for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
    int pin = pins[i];
    int total = 0;
    
    for (int j = 0; j < 3; j++) {
      total += analogRead(pin);
      delay(1);
    }
    
    int adcValue = total / 3;
    float voltage = adcValue * (3.3 / 4095.0);
    
    if (voltage > 1.5 && voltage < 2.5) {
      Serial.print(">>> НАЙДЕН GPIO");
      Serial.print(pin);
      Serial.print(": ");
      Serial.print(voltage, 2);
      Serial.print("V -> Батарея: ");
      Serial.print(voltage * 2.0, 2);
      Serial.println("V");
    } else if (voltage > 0.1) {
      Serial.print("GPIO");
      if (pin < 10) Serial.print("0");
      Serial.print(pin);
      Serial.print(": ");
      Serial.print(voltage, 2);
      Serial.println("V");
    }
    
    delay(30);
  }
  
  delay(3000);
}
