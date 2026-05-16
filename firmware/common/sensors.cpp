#include "sensors.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <HX711.h>

static Adafruit_SHT31 sht30;
static HX711 scale;

static bool _sht30Ok = false;
static bool _hx711Ok = false;

// ── ИНИЦИАЛИЗАЦИЯ ────────────────────────────────────────────────
bool setupSensors() {
    Wire.begin(I2C_SDA, I2C_SCL);

    // SHT30 (библиотека Adafruit SHT31 совместима)
    _sht30Ok = sht30.begin(SHT30_ADDRESS);
    if (!_sht30Ok) {
        Serial.printf("[SENSORS] SHT30 не найден (0x%02X)\n", SHT30_ADDRESS);
    } else {
        sht30.heater(false);
        Serial.println("[SENSORS] SHT30 OK");
    }

    // HX711
    scale.begin(HX711_DOUT, HX711_SCK);
    delay(400);
    _hx711Ok = scale.is_ready();
    if (!_hx711Ok) {
        Serial.println("[SENSORS] HX711 не готов");
    } else {
        scale.set_scale(HX711_CALIB);
        scale.tare();
        Serial.println("[SENSORS] HX711 OK");
    }

    return _sht30Ok && _hx711Ok;
}

// ── ЧТЕНИЕ ───────────────────────────────────────────────────────
SensorData readSensors() {
    SensorData d = {0, 0, 0, false, false};

    if (_sht30Ok) {
        d.temperature = sht30.readTemperature();
        d.humidity    = sht30.readHumidity();
        d.tempOk      = !isnan(d.temperature) && !isnan(d.humidity);
        if (!d.tempOk) {
            Serial.println("[SENSORS] SHT30 ошибка чтения");
            d.temperature = 0;
            d.humidity    = 0;
        }
    }

    if (_hx711Ok && scale.is_ready()) {
        d.weight   = scale.get_units(5); // среднее из 5 измерений
        d.weightOk = true;
        if (d.weight < 0) d.weight = 0;
    }

    Serial.printf("[SENSORS] T=%.1f°C  H=%.1f%%  W=%.3f кг\n",
                  d.temperature, d.humidity, d.weight);
    return d;
}

// ── КАЛИБРОВКА ВЕСОВ ─────────────────────────────────────────────
// Положи гирю известного веса, вызови функцию, запиши результат в config.h
bool calibrateScale(float knownWeightKg) {
    if (!_hx711Ok) return false;
    scale.set_scale();
    scale.tare();
    Serial.printf("[CALIB] Положи гирю %.1f кг и подожди 5 сек...\n", knownWeightKg);
    delay(5000);
    float reading = scale.get_units(10);
    float factor  = reading / knownWeightKg;
    Serial.printf("[CALIB] HX711_CALIB = %.2f  — запиши в config.h!\n", factor);
    scale.set_scale(factor);
    return true;
}

bool isSHT30Ready() { return _sht30Ok; }
bool isHX711Ready() { return _hx711Ok; }
