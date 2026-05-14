/*
 * Упрощенный тест для имеющихся компонентов
 * - MicroSD (запись/чтение)
 * - SHT30 (температура/влажность)
 * - A7670E (базовая проверка)
 * - HX711 (без датчиков - только проверка модуля)
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <SD.h>
#include <SPI.h>

// ===== GPIO PINS =====
// SHT30 I2C
#define I2C_SDA 21  // Желтый провод
#define I2C_SCL 22  // Зелёный провод

// HX711 (без датчиков)
#define HX711_DOUT 25
#define HX711_SCK 26

// MicroSD SPI
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

// A7670E
#define A7670_PWRKEY 4
#define A7670_RX 16
#define A7670_TX 17

// ===== Objects =====
Adafruit_SHT31 sht30;
HX711 scale;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n╔════════════════════════════════════╗");
  Serial.println("║  ТЕСТ ПЕРИФЕРИИ (упрощенный)      ║");
  Serial.println("╚════════════════════════════════════╝\n");

  Serial.println("Тестируем только то, что у вас есть:\n");
  
  // 1. Тест MicroSD
  testMicroSD();
  delay(1000);
  
  // 2. Тест SHT30
  testSHT30();
  delay(1000);
  
  // 3. Тест HX711 (без датчиков)
  testHX711_NoSensor();
  delay(1000);
  
  // 4. Тест A7670E (опционально - если есть питание)
  Serial.println("📡 ТЕСТ 4: A7670E LTE модем");
  Serial.println("─────────────────────────────────────");
  Serial.println("⚠️  Модему нужно питание 4V/2A!");
  Serial.println("   Пропустить тест модема? (y/n)");
  Serial.println("   (если нет аккумулятора - пропустите)\n");
  
  // Ждём 5 секунд на ответ
  unsigned long waitStart = millis();
  bool skipModem = true;
  while (millis() - waitStart < 5000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'n' || c == 'N') {
        skipModem = false;
        break;
      }
    }
  }
  
  if (skipModem) {
    Serial.println("   ⏭️  Тест модема пропущен");
    Serial.println("   💡 Протестируете когда будет аккумулятор\n");
  } else {
    testA7670E_Basic();
  }
  
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║       ТЕСТИРОВАНИЕ ЗАВЕРШЕНО       ║");
  Serial.println("╚════════════════════════════════════╝\n");
}

void loop() {
  // Простой мониторинг каждые 3 секунды
  delay(3000);
  
  Serial.println("\n───── Мониторинг ─────");
  
  // Температура и влажность
  float temp = sht30.readTemperature();
  float hum = sht30.readHumidity();
  if (!isnan(temp)) {
    Serial.printf("🌡️  %.1f°C  💧 %.1f%%\n", temp, hum);
  }
  
  // HX711 (без нагрузки будет показывать базовое значение)
  if (scale.is_ready()) {
    long val = scale.read();
    Serial.printf("⚖️  HX711: %ld\n", val);
  }
}

// ========================================
//         ТЕСТ 1: MicroSD
// ========================================
void testMicroSD() {
  Serial.println("📁 ТЕСТ 1: MicroSD накопитель");
  Serial.println("─────────────────────────────────");
  
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ Карта не найдена!");
    Serial.println("   • Вставьте карту");
    Serial.println("   • Проверьте подключение:");
    Serial.println("     CS   = IO5");
    Serial.println("     MOSI = IO23");
    Serial.println("     MISO = IO19");
    Serial.println("     SCK  = IO18\n");
    return;
  }
  
  Serial.println("✅ Карта обнаружена!");
  
  // Информация о карте
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("   📊 Размер: %llu MB\n", cardSize);
  
  uint64_t totalBytes = SD.totalBytes() / (1024 * 1024);
  uint64_t usedBytes = SD.usedBytes() / (1024 * 1024);
  Serial.printf("   💾 Занято: %llu MB из %llu MB\n", usedBytes, totalBytes);
  
  // Тест ЗАПИСИ
  Serial.println("\n   Тест записи...");
  File testFile = SD.open("/test_write.txt", FILE_WRITE);
  if (!testFile) {
    Serial.println("   ❌ Ошибка создания файла!");
    return;
  }
  
  testFile.println("=== ESP32 Test Data ===");
  testFile.println("Timestamp: " + String(millis()));
  testFile.println("Device: IoT Weight Monitor");
  testFile.close();
  Serial.println("   ✅ Запись успешна");
  
  // Тест ЧТЕНИЯ
  Serial.println("   Тест чтения...");
  testFile = SD.open("/test_write.txt", FILE_READ);
  if (!testFile) {
    Serial.println("   ❌ Ошибка открытия файла!");
    return;
  }
  
  Serial.println("   📄 Содержимое файла:");
  while (testFile.available()) {
    String line = testFile.readStringUntil('\n');
    Serial.println("      " + line);
  }
  testFile.close();
  Serial.println("   ✅ Чтение успешно");
  
  // Удаляем тестовый файл
  SD.remove("/test_write.txt");
  Serial.println("   🗑️  Тестовый файл удалён\n");
}

// ========================================
//         ТЕСТ 2: SHT30
// ========================================
void testSHT30() {
  Serial.println("🌡️  ТЕСТ 2: SHT30 (температура/влажность)");
  Serial.println("─────────────────────────────────");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  
  if (!sht30.begin(0x44)) {
    Serial.println("❌ Датчик не найден на адресе 0x44!");
    Serial.println("   Пробую адрес 0x45...");
    
    if (!sht30.begin(0x45)) {
      Serial.println("❌ Датчик не найден и на 0x45!");
      Serial.println("\n   Проверьте подключение:");
      Serial.println("   🔴 Красный  → 3.3V");
      Serial.println("   ⚫ Черный   → GND");
      Serial.println("   🟡 Желтый   → SDA (IO21)");
      Serial.println("   🟢 Зелёный  → SCL (IO22)");
      Serial.println("\n   💡 Если не работает - поменяйте");
      Serial.println("      желтый и зелёный местами!\n");
      return;
    }
  }
  
  Serial.println("✅ Датчик найден!");
  
  // Делаем несколько измерений
  delay(100);
  float temp = sht30.readTemperature();
  float hum = sht30.readHumidity();
  
  if (isnan(temp) || isnan(hum)) {
    Serial.println("❌ Ошибка чтения данных");
    return;
  }
  
  Serial.println("\n   📊 Измерения:");
  Serial.printf("   🌡️  Температура: %.1f°C\n", temp);
  Serial.printf("   💧 Влажность:    %.1f%%\n", hum);
  
  // Проверка разумности значений
  if (temp < -20 || temp > 60) {
    Serial.println("   ⚠️  Странная температура - возможна ошибка");
  }
  if (hum < 0 || hum > 100) {
    Serial.println("   ⚠️  Странная влажность - возможна ошибка");
  } else {
    Serial.println("   ✅ Значения в норме");
  }
  Serial.println();
}

// ========================================
//    ТЕСТ 3: HX711 (без датчиков)
// ========================================
void testHX711_NoSensor() {
  Serial.println("⚖️  ТЕСТ 3: HX711 АЦП (без датчиков)");
  Serial.println("─────────────────────────────────");
  
  scale.begin(HX711_DOUT, HX711_SCK);
  
  Serial.println("   Проверяю связь с модулем...");
  delay(500);
  
  if (!scale.is_ready()) {
    Serial.println("❌ HX711 не отвечает!");
    Serial.println("   Проверьте:");
    Serial.println("   • Питание модуля (3.3V или 5V)");
    Serial.println("   • DOUT → IO25");
    Serial.println("   • SCK  → IO26\n");
    return;
  }
  
  Serial.println("✅ Модуль отвечает!");
  
  // Читаем базовое значение (без датчиков)
  Serial.println("\n   📊 Базовые измерения (без нагрузки):");
  
  for (int i = 0; i < 5; i++) {
    if (scale.is_ready()) {
      long reading = scale.read();
      Serial.printf("   #%d: %ld\n", i+1, reading);
      delay(200);
    }
  }
  
  Serial.println("\n   💡 Без подключенных датчиков это");
  Serial.println("      просто базовое значение АЦП.");
  Serial.println("   ✅ Модуль работает корректно\n");
}

// ========================================
//         ТЕСТ 4: A7670E
// ========================================
void testA7670E_Basic() {
  Serial.println("📡 ТЕСТ 4: A7670E LTE модем");
  Serial.println("─────────────────────────────────");
  
  Serial.println("⚠️  ВАЖНО:");
  Serial.println("   • Micro USB НЕ НУЖЕН для работы!");
  Serial.println("   • Питание: VBAT → 4V от бустера");
  Serial.println("   • ОБЯЗАТЕЛЕН 470µF конденсатор на VBAT");
  Serial.println("   • Без конденсатора модем будет перезагружаться!\n");
  
  // Настройка UART
  Serial2.begin(115200, SERIAL_8N1, A7670_RX, A7670_TX);
  
  // Последовательность включения через PWRKEY
  pinMode(A7670_PWRKEY, OUTPUT);
  Serial.println("   🔌 Включаю модем через PWRKEY...");
  
  digitalWrite(A7670_PWRKEY, LOW);
  delay(100);
  digitalWrite(A7670_PWRKEY, HIGH);
  delay(1000);
  digitalWrite(A7670_PWRKEY, LOW);
  
  Serial.println("   ⏳ Жду инициализацию (10 сек)...");
  delay(10000);
  
  // Очистка буфера
  while (Serial2.available()) Serial2.read();
  
  // Тест команды AT
  Serial.println("   📤 Отправляю AT...");
  Serial2.println("AT");
  delay(1000);
  
  String response = "";
  while (Serial2.available()) {
    response += (char)Serial2.read();
  }
  
  if (response.indexOf("OK") >= 0) {
    Serial.println("   ✅ Модем отвечает!");
    Serial.println("\n   📋 Ответ модема:");
    Serial.println("   " + response);
    
    // Пробуем узнать модель
    Serial2.println("AT+CGMM");
    delay(500);
    Serial.print("   📱 Модель: ");
    while (Serial2.available()) {
      Serial.write(Serial2.read());
    }
    
  } else {
    Serial.println("   ❌ Модем не отвечает!");
    Serial.println("\n   Возможные причины:");
    Serial.println("   1. НЕТ КОНДЕНСАТОРА 470µF → модем перезагружается");
    Serial.println("   2. Недостаточное питание (нужно 4V, 2A)");
    Serial.println("   3. Перепутаны RX/TX (должны быть перекрёстно)");
    Serial.println("   4. Модем ещё не включился (подождите дольше)");
    
    if (response.length() > 0) {
      Serial.println("\n   📋 Получен ответ:");
      Serial.println("   " + response);
    }
  }
  Serial.println();
}
