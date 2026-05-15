#!/bin/bash
# Final structure: .cpp only (except config.h)

cd /d/NextCloud/3D_NAS_Code/BeeHive/V2/dev

echo "Creating structure..."

# Clean
rm -rf src/ migrate.sh firmware/

# === COMMON ===
mkdir -p firmware/common

cat > firmware/common/config.h << 'HEOF'
#ifndef CONFIG_H
#define CONFIG_H

// GPIO Pins
#define I2C_SDA 21
#define I2C_SCL 22
#define HX711_DOUT 25
#define HX711_SCK 26
#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define A7670_PWRKEY 4
#define A7670_RX 16
#define A7670_TX 17
#define BATTERY_ADC 34
#define BUTTON_PIN 13

// Calibration
#define HX711_CALIB -7050.0f
#define BATTERY_DIVIDER 2.0f

// Timing
#define SLEEP_TIME 3600  // seconds

// Telegram
#define TELEGRAM_TOKEN "YOUR_TOKEN"
#define TELEGRAM_CHAT "YOUR_CHAT_ID"
#define APN_NAME "internet"

#endif
HEOF

cat > firmware/common/sensors.cpp << 'CEOF'
// SHT30 temperature/humidity
// HX711 weight with calibration
#include <Arduino.h>

void setupSensors() {
    // TODO: I2C, SHT30, HX711 setup
}

void readSensors() {
    // TODO: read temp, humidity, weight
}

float getTemperature() {
    // TODO
    return 0.0;
}

float getHumidity() {
    // TODO
    return 0.0;
}

float getWeight() {
    // TODO
    return 0.0;
}

void calibrateScale() {
    // TODO: HX711 calibration
}
CEOF

cat > firmware/common/sd_card.cpp << 'CEOF'
// MicroSD card logging
#include <Arduino.h>

void setupSD() {
    // TODO: SPI, SD.begin()
}

void logToSD(float temp, float hum, float weight, float battery) {
    // TODO: append to CSV
}

void readFromSD() {
    // TODO: read logs
}
CEOF

cat > firmware/common/battery.cpp << 'CEOF'
// Battery voltage reading
#include <Arduino.h>

void setupBattery() {
    // TODO: ADC setup
}

float readBattery() {
    // TODO: read ADC, voltage divider
    return 0.0;
}

int getBatteryPercent() {
    // TODO: voltage to %
    return 0;
}
CEOF

cat > firmware/common/bot.cpp << 'CEOF'
// Telegram bot (works over GSM or WiFi)
#include <Arduino.h>

void sendTelegram(String message) {
    // TODO: HTTP POST to Telegram API
    // Can use TinyGsmClient or WiFiClient
}

void formatMessage(float temp, float hum, float weight, float battery) {
    // TODO: create formatted message
}
CEOF

# === GSM ===
mkdir -p firmware/gsm

cat > firmware/gsm/main.cpp << 'CEOF'
// GSM version main
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("BEE HIVE - GSM");
    
    // TODO:
    // - setupSensors()
    // - setupSD()
    // - setupBattery()
    // - setupModem()
    // - readSensors()
    // - logToSD()
    // - sendTelegram()
    // - getGPS() (once per day)
    // - goToSleep()
}

void loop() {
    // Empty - using deep sleep
}
CEOF

cat > firmware/gsm/modem.cpp << 'CEOF'
// A7670E modem control
#include <Arduino.h>

void setupModem() {
    // TODO:
    // - Serial2 init
    // - PWRKEY sequence
    // - AT commands init
    // - GPRS connect
}

void powerOnModem() {
    // TODO: PWRKEY sequence
}

void powerOffModem() {
    // TODO: power down
}

bool connectGPRS() {
    // TODO: APN connect
    return false;
}

int getSignalStrength() {
    // TODO: AT+CSQ
    return 0;
}
CEOF

cat > firmware/gsm/gps.cpp << 'CEOF'
// GPS from A7670E
#include <Arduino.h>

void setupGPS() {
    // TODO: enable GPS in modem
}

void getGPS() {
    // TODO: read coordinates
    // Only once per day!
}

float getLatitude() {
    return 0.0;
}

float getLongitude() {
    return 0.0;
}
CEOF

# === WIFI ===
mkdir -p firmware/wifi

cat > firmware/wifi/main.cpp << 'CEOF'
// WiFi version main
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("BEE HIVE - WiFi");
    
    // TODO:
    // - setupSensors()
    // - setupSD()
    // - setupBattery()
    // - connectWiFi()
    // - readSensors()
    // - logToSD()
    // - sendTelegram()
    // - goToSleep()
}

void loop() {
    // Check button for config portal
}
CEOF

cat > firmware/wifi/wifi_module.cpp << 'CEOF'
// WiFi connection
#include <Arduino.h>

void setupWiFi() {
    // TODO: WiFi.begin()
}

void connectWiFi() {
    // TODO: connect to saved SSID
}

void disconnectWiFi() {
    // TODO: disconnect
}

bool isConnected() {
    return false;
}
CEOF

cat > firmware/wifi/server.cpp << 'CEOF'
// Web server for data display
#include <Arduino.h>

void startWebServer() {
    // TODO: AsyncWebServer
    // Show current data via web interface
}

void handleRoot() {
    // TODO: HTML page with data
}

void handleAPI() {
    // TODO: JSON API endpoint
}
CEOF

cat > firmware/wifi/portal.cpp << 'CEOF'
// Captive portal for WiFi setup
#include <Arduino.h>

void startConfigPortal() {
    // TODO:
    // - Create AP "BeeHive-Setup"
    // - DNS server for captive portal
    // - Web form for SSID/password
    // - Save to EEPROM/Flash
}

void handleConfigSave() {
    // TODO: save WiFi credentials
}
CEOF

# === DOCS ===
mkdir -p docs
echo "# Documentation" > docs/README.md

# === HARDWARE ===
mkdir -p hardware
echo "# Hardware (BOM, schematics)" > hardware/README.md

# Commit
git add .
git commit -m "feat: modular structure (cpp only)

Structure:
- common: sensors, sd_card, battery, bot, config.h
- gsm: main, modem, gps
- wifi: main, wifi_module, server, portal

All .cpp files (no .h except config.h)
Ready for implementation"

echo ""
echo "✅ Structure created!"
echo ""
echo "Files:"
find firmware/ -name "*.cpp" -o -name "*.h" | sort
