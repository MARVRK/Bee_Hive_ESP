#include "lcd_screen.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
static bool _displayOk = false;

// ── ИНИЦИАЛИЗАЦИЯ ────────────────────────────────────────────────
bool setupDisplay() {
    _displayOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (!_displayOk) {
        Serial.printf("[OLED] Не найден (0x%02X)\n", OLED_ADDRESS);
        return false;
    }
    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    Serial.println("[OLED] OK");
    return true;
}

bool isDisplayReady() { return _displayOk; }

// ── ЗАГРУЗОЧНЫЙ ЭКРАН ────────────────────────────────────────────
void showBootScreen() {
    if (!_displayOk) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20, 10);
    display.println("BEE HIVE V2");
    display.setCursor(28, 28);
    display.println("Starting...");

    #ifdef VERSION_WIFI
    display.setCursor(36, 46);
    display.println("[WiFi]");
    #else
    display.setCursor(38, 46);
    display.println("[GSM]");
    #endif

    display.display();
}

// ── ОСНОВНОЙ ЭКРАН ДАННЫХ ────────────────────────────────────────
// Макет (128×64):
//   BEE HIVE          10:30
//   Вес:  45.320 кг
//   T: 24.5°C  H: 68%
//   Бат: [=======  ] 78%
void showData(const SensorData& s, const BatteryData& b, const String& time) {
    if (!_displayOk) return;
    display.clearDisplay();
    display.setTextSize(1);

    // Строка 1: заголовок + время
    display.setCursor(0, 0);
    display.print("BEE HIVE");
    if (time.length() > 0) {
        display.setCursor(128 - (time.length() * 6), 0);
        display.print(time);
    }

    // Строка 2: вес
    display.setCursor(0, 16);
    char wStr[24];
    snprintf(wStr, sizeof(wStr), "Вес: %.3f кг", s.weight);
    display.print(wStr);

    // Строка 3: температура и влажность
    display.setCursor(0, 32);
    char tStr[24];
    snprintf(tStr, sizeof(tStr), "T:%.1fC  H:%.0f%%", s.temperature, s.humidity);
    display.print(tStr);

    // Строка 4: батарея (прогресс-бар)
    display.setCursor(0, 48);
    display.print("Bat:");
    // Рисуем прогресс бар [          ]
    int barX = 26, barY = 48, barW = 72, barH = 8;
    display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
    int fill = (int)((barW - 2) * b.percent / 100.0f);
    if (fill > 0) display.fillRect(barX + 1, barY + 1, fill, barH - 2, SSD1306_WHITE);
    char pStr[6];
    snprintf(pStr, sizeof(pStr), "%d%%", b.percent);
    display.setCursor(102, 48);
    display.print(pStr);

    display.display();
}

// ── СООБЩЕНИЕ (3 строки) ─────────────────────────────────────────
void showMessage(const String& line1, const String& line2, const String& line3) {
    if (!_displayOk) return;
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.println(line1);
    if (line2.length()) { display.setCursor(0, 28); display.println(line2); }
    if (line3.length()) { display.setCursor(0, 46); display.println(line3); }
    display.display();
}

// ── ВЫКЛЮЧИТЬ ДИСПЛЕЙ ────────────────────────────────────────────
void turnOffDisplay() {
    if (!_displayOk) return;
    display.ssd1306_command(SSD1306_DISPLAYOFF);
}
