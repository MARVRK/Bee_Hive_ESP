// BEE HIVE V2 — WiFi Firmware
#include <Arduino.h>
#include <esp_sleep.h>
#include "../common/config.h"
#include "../common/sensors.h"
#include "../common/battery.h"
#include "../common/sd_card.h"
#include "../common/lcd_screen.h"
#include "../common/bot.h"
#include "wifi_module.h"
#include "portal.h"
#include "server.h"

// ── СОСТОЯНИЕ МЕЖДУ ЦИКЛАМИ (RTC память — переживает deep sleep) ──
RTC_DATA_ATTR int  bootCount      = 0;
RTC_DATA_ATTR int  sleepTimeSec   = SLEEP_TIME_SEC;
RTC_DATA_ATTR bool notifEnabled   = true;
RTC_DATA_ATTR int  sendFailures   = 0;
RTC_DATA_ATTR float lastLat       = 0;
RTC_DATA_ATTR float lastLon       = 0;
RTC_DATA_ATTR bool  locFresh      = false;

// ── ПРОВЕРКА ПЕРИФЕРИИ ────────────────────────────────────────────
struct HWStatus {
    bool sensors;
    bool sdCard;
    bool battery;
    bool display;
    bool wifi;
};

HWStatus checkPeripherals() {
    HWStatus hw = {false, false, false, false, false};
    Serial.println("\n=== PERIPHERAL CHECK ===");

    hw.sensors = setupSensors();
    Serial.printf("  [SENSORS] %s\n", hw.sensors ? "OK" : "FAIL");

    hw.sdCard = setupSD();
    Serial.printf("  [SD    ] %s\n", hw.sdCard ? "OK" : "FAIL");

    setupBattery();
    hw.battery = (readVoltage() > 2.5f); // есть хоть какое-то напряжение
    Serial.printf("  [BAT   ] %s\n", hw.battery ? "OK" : "FAIL");

    hw.display = setupDisplay();
    Serial.printf("  [OLED  ] %s\n", hw.display ? "OK" : "FAIL");

    hw.wifi = setupWiFi();
    Serial.printf("  [WiFi  ] %s\n", hw.wifi ? "Init OK" : "FAIL");

    Serial.println("========================\n");
    return hw;
}

// ── ПРОВЕРКА КНОПКИ ──────────────────────────────────────────────
// Возвращает: 0 = нет нажатия, 1 = короткое, 2 = долгое
static int checkButton(int pin) {
    if (digitalRead(pin) != LOW) return 0;
    unsigned long t = millis();
    while (digitalRead(pin) == LOW && millis() - t < BTN_LONG_PRESS + 100) delay(10);
    return (millis() - t >= BTN_LONG_PRESS) ? 2 : 1;
}

// ── ТЕКУЩЕЕ ВРЕМЯ (заглушка до DS3231) ───────────────────────────
static String getTimeStr() {
    // Пока DS3231 не куплен — возвращаем uptime
    unsigned long s = millis() / 1000;
    char buf[16];
    snprintf(buf, sizeof(buf), "+%lus", s);
    return String(buf);
}

// ── SLEEP ─────────────────────────────────────────────────────────
static void goToSleep(int seconds) {
    Serial.printf("[SLEEP] Сон на %d сек (boot #%d)\n\n", seconds, bootCount);
    Serial.flush();
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    // Кнопка экрана тоже будит из deep sleep
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_SCREEN, 0);
    esp_deep_sleep_start();
}

// ── SETUP ─────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(100);
    bootCount++;

    Serial.printf("\n>>> BEE HIVE V2 — WiFi | Boot #%d <<<\n", bootCount);

    // Настройка пинов кнопок
    pinMode(BUTTON_SCREEN, INPUT_PULLUP);
    pinMode(BUTTON_WIFI,   INPUT_PULLUP);

    // ── Причина пробуждения ────────────────────────────────────────
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_EXT0) {
        // Проснулись по кнопке экрана — только показать данные и обратно спать
        Serial.println("[WAKE] Кнопка экрана");
        if (setupDisplay()) {
            setupBattery();
            setupSensors();
            SensorData s  = readSensors();
            BatteryData b = readBattery();
            showData(s, b, getTimeStr());
            delay(OLED_TIMEOUT_MS);
            turnOffDisplay();
        }
        goToSleep(sleepTimeSec);
        return;
    }

    // ── Обычное пробуждение по таймеру ────────────────────────────

    // Загружаем расписание из NVS (пользователь мог изменить через бот)
    sleepTimeSec = loadSchedule();

    HWStatus hw = checkPeripherals();

    if (hw.display) showBootScreen();

    // Если сенсоры недоступны — нет смысла в цикле
    if (!hw.sensors) {
        Serial.println("[WARN] Сенсоры не отвечают — пропускаем цикл");
        showMessage("WARN", "Sensors fail", "Skip cycle");
        delay(2000);
        goToSleep(sleepTimeSec);
        return;
    }

    // ── 1. Чтение данных ──────────────────────────────────────────
    SensorData  sensors = readSensors();
    BatteryData battery = readBattery();
    String      timeStr = getTimeStr();

    // Критически низкий заряд — сон до лучших времён
    if (battery.percent < 3) {
        Serial.println("[WARN] Батарея критически низкая!");
        showMessage("LOW BATTERY", String(battery.percent) + "%", "Sleep...");
        delay(2000);
        goToSleep(sleepTimeSec * 4); // спим дольше чтобы сэкономить
        return;
    }

    // ── 2. Запись на SD ───────────────────────────────────────────
    if (hw.sdCard) {
        LogEntry entry;
        entry.timestamp    = timeStr;
        entry.sensors      = sensors;
        entry.battery      = battery;
        entry.lat          = lastLat;
        entry.lon          = lastLon;
        entry.locationFresh = locFresh;
        entry.signalDbm    = 0; // заполним после подключения
        entry.connType     = "WiFi";
        entry.sendFailures = sendFailures;
        logToSD(entry);
    }

    // ── 3. Обновляем OLED если нажата кнопка экрана ───────────────
    if (hw.display) {
        showData(sensors, battery, timeStr);
        // Дисплей выключится сам через OLED_TIMEOUT_MS
    }

    // ── 4. WiFi — подключение ─────────────────────────────────────
    // Проверяем кнопку настройки (долгое нажатие)
    if (checkButton(BUTTON_WIFI) == 2) {
        Serial.println("[WIFI] Запуск портала настройки...");
        showMessage("WiFi Setup", PORTAL_SSID, "Connect phone");
        startConfigPortal(); // restart внутри если сохранено
    }

    bool wifiOk = connectWiFi();

    if (!wifiOk) {
        sendFailures++;
        Serial.printf("[WARN] WiFi недоступен. Failures: %d\n", sendFailures);
        showMessage("WiFi FAIL", "No connection", String(sendFailures) + " fails");
        delay(2000);
        goToSleep(sleepTimeSec);
        return;
    }

    // ── 5. Отправка телеметрии ────────────────────────────────────
    bool sent = sendTelemetryNow(sensors, battery, lastLat, lastLon, locFresh, timeStr);
    if (sent) {
        sendFailures = 0;
    } else {
        sendFailures++;
    }

    // ── 6. Ожидание команд (30 сек long polling) ──────────────────
    pollTelegramCommands(sensors, battery, lastLat, lastLon, locFresh,
                         timeStr, sleepTimeSec);

    // ── 7. Отключаемся ───────────────────────────────────────────
    disconnectWiFi();

    // ── 8. Гасим дисплей и уходим спать ──────────────────────────
    if (hw.display) {
        delay(OLED_TIMEOUT_MS); // показываем данные пока не погаснет
        turnOffDisplay();
    }

    goToSleep(sleepTimeSec);
}

void loop() {
    // Пусто — используем deep sleep
}
