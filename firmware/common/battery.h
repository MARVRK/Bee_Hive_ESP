#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

struct BatteryData {
    float voltage;        // В
    int   percent;        // 0–100
    int   estimatedHours; // расчётный остаток
};

void        setupBattery();
BatteryData readBattery();
float       readVoltage();
int         voltageToPercent(float voltage);

#endif
