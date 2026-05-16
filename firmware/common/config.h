#ifndef CONFIG_H
#define CONFIG_H

// ── GPIO PINS ────────────────────────────────────────────────────

// I2C (OLED + DS3231 + SHT30)
#define I2C_SDA         21
#define I2C_SCL         22

// HX711 (весовые датчики)
#define HX711_DOUT      25
#define HX711_SCK       26

// MicroSD (SPI)
#define SD_CS           5
#define SD_MOSI         23
#define SD_MISO         19
#define SD_SCK          18

// GSM A7670E
#define A7670_PWRKEY    4
#define A7670_RX        16
#define A7670_TX        17

// Батарея (делитель 100кОм + 100кОм)
#define BATTERY_ADC     34

// Кнопки
#define BUTTON_SCREEN   13   // активация OLED на 30 сек
#define BUTTON_WIFI     33   // долгое нажатие = режим настройки WiFi

// DS3231 RTC прерывание (будильник → пробуждение из deep sleep)
#define RTC_INT_PIN     35

// ── I2C АДРЕСА ──────────────────────────────────────────────────
#define OLED_ADDRESS    0x3C   // SSD1306 (или 0x3D)
#define DS3231_ADDRESS  0x68
#define SHT30_ADDRESS   0x44   // (или 0x45)
#define AT24C32_ADDRESS 0x57   // EEPROM на модуле DS3231

// ── OLED ────────────────────────────────────────────────────────
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_TIMEOUT_MS 30000  // 30 секунд — потом гаснет

// ── КАЛИБРОВКА ──────────────────────────────────────────────────
#define HX711_CALIB     -7050.0f   // подстроить при первом запуске
#define BATTERY_DIVIDER 2.0f       // коэффициент делителя напряжения

// ── БАТАРЕЯ (18650 LiIon) ────────────────────────────────────────
#define BAT_VOLTAGE_MAX 4.2f   // 100%
#define BAT_VOLTAGE_MIN 3.0f   // 0%
#define BAT_CAPACITY_MAH 7000  // 2× 18650 в параллели

// ── РАСПИСАНИЕ / СОН ────────────────────────────────────────────
#define SLEEP_TIME_SEC  3600   // по умолчанию: раз в час
#define SLEEP_MIN_SEC   900    // минимум: 15 минут

// ── SD КАРТА ────────────────────────────────────────────────────
#define SD_LOG_DIR      "/data"
#define SD_MAX_DAYS     90     // циклическая запись: 90 дней

// ── TELEGRAM ────────────────────────────────────────────────────
// Токены — в include/secrets.h (не в git!)
#define TELEGRAM_API    "https://api.telegram.org"
#define TELEGRAM_TIMEOUT 30   // секунд ожидания команд (long polling)

// ── WiFi PORTAL ─────────────────────────────────────────────────
#define PORTAL_SSID     "BeeHive-Setup"
#define PORTAL_TIMEOUT  30000  // 30 сек без активности → выход

// ── КНОПКА ──────────────────────────────────────────────────────
#define BTN_LONG_PRESS  3000   // 3 сек = долгое нажатие

#endif // CONFIG_H
