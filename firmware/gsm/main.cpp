// BEE HIVE V2 — GSM Firmware
#include <Arduino.h>
#include "../common/config.h"

// Forward declarations (модули будут подключены по мере готовности)
void setupSensors();
void setupSD();
void setupBattery();
void setupModem();
void readSensors();
void logToSD(float temp, float hum, float weight, float battery);
void sendTelegram(String message);
void goToSleep(int seconds);

// ─────────────────────────────────────────
// ПРОВЕРКА ПЕРИФЕРИИ
// Запускается один раз при каждом старте.
// Проверяет все модули и выводит статус в Serial.
// Не блокирует работу — только логирует.
// ─────────────────────────────────────────
struct PeriphStatus {
    bool sensors = false;   // SHT30 + HX711
    bool sdCard  = false;   // MicroSD
    bool battery = false;   // АЦП делитель
    bool modem   = false;   // A7670E
    bool display = false;   // OLED
};

PeriphStatus checkPeripherals() {
    PeriphStatus st;
    Serial.println("\n=== PERIPHERAL CHECK ===");

    // --- SHT30 + HX711 ---
    // TODO: реальная проверка через Wire.begin() + Wire.endTransmission()
    // и HX711.is_ready()
    Serial.print("[SENSORS] SHT30 + HX711 ... ");
    st.sensors = false; // заменить на результат проверки
    Serial.println(st.sensors ? "OK" : "FAIL (TODO)");

    // --- MicroSD ---
    // TODO: SD.begin(SD_CS)
    Serial.print("[SD    ] MicroSD      ... ");
    st.sdCard = false;
    Serial.println(st.sdCard ? "OK" : "FAIL (TODO)");

    // --- Battery ADC ---
    // TODO: analogRead(BATTERY_ADC) > 0
    Serial.print("[BAT   ] Battery ADC  ... ");
    st.battery = false;
    Serial.println(st.battery ? "OK" : "FAIL (TODO)");

    // --- OLED ---
    // TODO: Wire.beginTransmission(0x3C) == 0
    Serial.print("[OLED  ] Display      ... ");
    st.display = false;
    Serial.println(st.display ? "OK" : "FAIL (TODO)");

    // --- A7670E GSM модем ---
    // TODO: отправить AT, ждать OK
    Serial.print("[MODEM ] A7670E       ... ");
    st.modem = false;
    Serial.println(st.modem ? "OK" : "FAIL (TODO)");

    Serial.println("========================\n");
    return st;
}

// ─────────────────────────────────────────
// SETUP — точка входа после пробуждения
// ─────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n\n>>> BEE HIVE V2 — GSM <<<");

    // Проверка периферии при каждом старте
    PeriphStatus hw = checkPeripherals();

    // Если критичные модули недоступны — уходим спать и пробуем позже
    if (!hw.sensors) {
        Serial.println("[WARN] Sensors unavailable, skipping cycle");
        goToSleep(SLEEP_TIME);
        return;
    }

    // ── Основной цикл GSM ──────────────────
    // 1. Читаем данные
    readSensors();

    // 2. Пишем на SD (если доступна)
    if (hw.sdCard) {
        logToSD(0, 0, 0, 0); // TODO: передать реальные значения
    } else {
        Serial.println("[WARN] SD unavailable, data not logged");
    }

    // 3. GSM — включаем модем и отправляем
    if (hw.modem) {
        setupModem();
        sendTelegram("TODO: сформировать сообщение");
    } else {
        Serial.println("[WARN] Modem unavailable, skipping send");
    }

    // 4. Сон до следующего цикла
    goToSleep(SLEEP_TIME);
}

void loop() {
    // Пусто — используем deep sleep
}

// ─────────────────────────────────────────
// SLEEP
// ─────────────────────────────────────────
void goToSleep(int seconds) {
    Serial.printf("[SLEEP] Going to sleep for %d sec\n", seconds);
    Serial.flush();
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}
