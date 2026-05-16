#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

struct SensorData {
    float temperature;   // °C
    float humidity;      // %
    float weight;        // кг
    bool  tempOk;
    bool  weightOk;
};

bool        setupSensors();
SensorData  readSensors();
bool        calibrateScale(float knownWeightKg);
bool        isSHT30Ready();
bool        isHX711Ready();

#endif
