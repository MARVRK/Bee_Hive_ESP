/*
 * Bee Hive ESP32 Monitor
 * Мониторинг веса и климата улья с отправкой данных в Telegram
 * 
 * Hardware:
 * - ESP32-WROOM-32D
 * - A7670E-FASE (4G LTE)
 * - SHT30 (температура/влажность)
 * - HX711 + 4x50kg load cells
 * - MicroSD card
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <SD.h>
#include <SPI.h>

// Подключаем секреты (WiFi, Telegram токены и т.д.)
#ifdef USE_SECRETS
#include "secrets.h"
#endif

// ===== GPIO CONFIGURATION =====
// A7670E 4G Modem
#define A7670_PWRKEY    4
#define A7670_RX        16
#define A7670_TX        17

// I2C для SHT30
#define I2C_SDA         21
#define I2C_SCL         22

// HX711 Load Cell Amplifier
#define HX711_DOUT      25
#define HX711_SCK       26

// SPI для MicroSD
#define SD_CS           5
#define SD_MOSI         23
#define SD_MISO         19
#define SD_SCK          18

// Battery voltage monitor
#define BATTERY_ADC     34

// ===== CALIBRATION VALUES =====
#ifndef HX711_CALIBRATION_FACTOR
#define HX711_CALIBRATION_FACTOR -7050  // Подберите свой
#endif

#ifndef HX711_OFFSET
#define HX711_OFFSET 0L
#endif

// ===== CONFIGURATION =====
#define MEASUREMENT_INTERVAL 60000  // 60 секунд между измерениями
#define TELEGRAM_UPDATE_INTERVAL 3600000  // 1 час между отправками в Telegram

// ===== OBJECTS =====
Adafruit_SHT31 sht30;
HX711 scale;

// ===== GLOBAL VARIABLES =====
unsigned long lastMeasurement = 0;
unsigned long lastTelegramUpdate = 0;

struct SensorData {
  float weight;
  float temperature;
  float humidity;
  float batteryVoltage;
  unsigned long timestamp;
};

SensorData currentData;

// ===== FUNCTION PROTOTYPES =====
void setupSensors();
void setupModem();
void setupSD();
void readSensors();
void logToSD();
void sendToTelegram();
float readBatteryVoltage();

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n╔════════════════════════════════════╗");
  Serial.println("║    🐝 BEE HIVE MONITOR v1.0       ║");
  Serial.println("╚════════════════════════════════════╝\n");
  
  // Инициализация компонентов
  Serial.println("Инициализация...");
  
  setupSensors();
  setupSD();
  setupModem();
  
  Serial.println("\n✅ Система готова к работе!\n");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Периодические измерения
  if (currentMillis - lastMeasurement >= MEASUREMENT_INTERVAL) {
    lastMeasurement = currentMillis;
    
    Serial.println("─── Новое измерение ───");
    readSensors();
    logToSD();
    
    // Отправка в Telegram по расписанию
    if (currentMillis - lastTelegramUpdate >= TELEGRAM_UPDATE_INTERVAL) {
      lastTelegramUpdate = currentMillis;
      sendToTelegram();
    }
  }
  
  delay(100);
}

// ========================================
//         ИНИЦИАЛИЗАЦИЯ ДАТЧИКОВ
// ========================================

void setupSensors() {
  Serial.print("🌡️  SHT30... ");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  
  if (!sht30.begin(0x44)) {
    Serial.println("❌ Не найден!");
    Serial.println("   Попробуйте адрес 0x45 или проверьте подключение");
  } else {
    Serial.println("✅ OK");
  }
  
  Serial.print("⚖️  HX711... ");
  scale.begin(HX711_DOUT, HX711_SCK);
  
  if (scale.is_ready()) {
    scale.set_scale(HX711_CALIBRATION_FACTOR);
    scale.tare();
    Serial.println("✅ OK");
  } else {
    Serial.println("❌ Не отвечает!");
  }
  
  // Настройка ADC для батареи
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void setupModem() {
  Serial.print("📡 A7670E... ");
  
  Serial2.begin(115200, SERIAL_8N1, A7670_RX, A7670_TX);
  
  // Power on sequence
  pinMode(A7670_PWRKEY, OUTPUT);
  digitalWrite(A7670_PWRKEY, LOW);
  delay(100);
  digitalWrite(A7670_PWRKEY, HIGH);
  delay(1000);
  digitalWrite(A7670_PWRKEY, LOW);
  
  Serial.println("⏳ Включение...");
  delay(5000);
  
  // TODO: Добавить инициализацию модема
  Serial.println("   (настройка модема будет добавлена)");
}

void setupSD() {
  Serial.print("💾 MicroSD... ");
  
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ Не найдена!");
    return;
  }
  
  Serial.println("✅ OK");
  
  // Создаем файл лога если его нет
  if (!SD.exists("/bee_log.csv")) {
    File logFile = SD.open("/bee_log.csv", FILE_WRITE);
    if (logFile) {
      logFile.println("Timestamp,Weight(kg),Temperature(C),Humidity(%),Battery(V)");
      logFile.close();
      Serial.println("   📝 Создан новый лог файл");
    }
  }
}

// ========================================
//            ЧТЕНИЕ ДАННЫХ
// ========================================

void readSensors() {
  // Температура и влажность
  currentData.temperature = sht30.readTemperature();
  currentData.humidity = sht30.readHumidity();
  
  // Вес
  if (scale.is_ready()) {
    currentData.weight = scale.get_units(5);  // Среднее из 5 измерений
  } else {
    currentData.weight = -1;
  }
  
  // Напряжение батареи
  currentData.batteryVoltage = readBatteryVoltage();
  
  // Timestamp
  currentData.timestamp = millis();
  
  // Вывод в Serial
  Serial.printf("⚖️  Вес:        %.2f кг\n", currentData.weight);
  Serial.printf("🌡️  Температура: %.1f°C\n", currentData.temperature);
  Serial.printf("💧 Влажность:   %.1f%%\n", currentData.humidity);
  Serial.printf("🔋 Батарея:     %.2fV\n", currentData.batteryVoltage);
  Serial.println();
}

float readBatteryVoltage() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(BATTERY_ADC);
    delay(10);
  }
  int avg = sum / 10;
  
  // Voltage divider: 100k + 100k = x2
  return (avg / 4095.0) * 3.3 * 2.0;
}

// ========================================
//          ЛОГИРОВАНИЕ НА SD
// ========================================

void logToSD() {
  File logFile = SD.open("/bee_log.csv", FILE_APPEND);
  
  if (!logFile) {
    Serial.println("❌ Ошибка записи на SD");
    return;
  }
  
  // Формат: Timestamp,Weight,Temperature,Humidity,Battery
  logFile.print(currentData.timestamp);
  logFile.print(",");
  logFile.print(currentData.weight, 2);
  logFile.print(",");
  logFile.print(currentData.temperature, 1);
  logFile.print(",");
  logFile.print(currentData.humidity, 1);
  logFile.print(",");
  logFile.println(currentData.batteryVoltage, 2);
  
  logFile.close();
  
  Serial.println("💾 Данные записаны на SD");
}

// ========================================
//         ОТПРАВКА В TELEGRAM
// ========================================

void sendToTelegram() {
  Serial.println("📱 Отправка в Telegram...");
  
  // TODO: Реализовать отправку через A7670E
  // - Подключиться к сети
  // - Отправить HTTP POST к Telegram Bot API
  // - Форматированное сообщение с данными
  
  Serial.println("   (функция будет реализована)");
}
