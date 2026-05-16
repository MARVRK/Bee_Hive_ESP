#include "battery.h"
#include "config.h"

// ── ИНИЦИАЛИЗАЦИЯ ────────────────────────────────────────────────
void setupBattery() {
    // ESP32 ADC: 12 бит, 0–3.3V
    // IO34 — input only, без pull-up
    analogSetAttenuation(ADC_11db); // диапазон 0–3.3V
    analogSetWidth(12);             // 12 бит = 0–4095
    Serial.println("[BAT] ADC инициализирован (IO34)");
}

// ── НАПРЯЖЕНИЕ ───────────────────────────────────────────────────
float readVoltage() {
    // Несколько замеров для усреднения (ADC ESP32 нелинеен)
    long sum = 0;
    const int samples = 16;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(BATTERY_ADC);
        delay(2);
    }
    float raw = (float)sum / samples;

    // raw (0–4095) → напряжение на пине (0–3.3V)
    float vPin = raw / 4095.0f * 3.3f;

    // Учитываем делитель (100кОм + 100кОм = ÷2)
    float vBat = vPin * BATTERY_DIVIDER;

    return vBat;
}

// ── ПРОЦЕНТ ──────────────────────────────────────────────────────
// Кривая разряда LiIon 18650
int voltageToPercent(float v) {
    if (v >= 4.20f) return 100;
    if (v >= 4.10f) return 90 + (int)((v - 4.10f) / 0.10f * 10);
    if (v >= 4.00f) return 80 + (int)((v - 4.00f) / 0.10f * 10);
    if (v >= 3.85f) return 65 + (int)((v - 3.85f) / 0.15f * 15);
    if (v >= 3.70f) return 45 + (int)((v - 3.70f) / 0.15f * 20);
    if (v >= 3.55f) return 25 + (int)((v - 3.55f) / 0.15f * 20);
    if (v >= 3.40f) return 10 + (int)((v - 3.40f) / 0.15f * 15);
    if (v >= 3.20f) return 3  + (int)((v - 3.20f) / 0.20f * 7);
    if (v >= 3.00f) return 1;
    return 0;
}

// ── РАСЧЁТ ОСТАТКА ───────────────────────────────────────────────
// Среднее потребление: ~20 мА (deep sleep + периодическая работа)
static int estimateHours(int percent) {
    // 7000 мАч × % / 100 / 20 мА = часы
    return (BAT_CAPACITY_MAH * percent / 100) / 20;
}

// ── ПОЛНОЕ ЧТЕНИЕ ────────────────────────────────────────────────
BatteryData readBattery() {
    BatteryData b;
    b.voltage        = readVoltage();
    b.percent        = voltageToPercent(b.voltage);
    b.estimatedHours = estimateHours(b.percent);

    Serial.printf("[BAT] %.2fV  %d%%  ~%d ч\n",
                  b.voltage, b.percent, b.estimatedHours);
    return b;
}
