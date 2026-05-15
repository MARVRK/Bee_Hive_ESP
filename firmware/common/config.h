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
