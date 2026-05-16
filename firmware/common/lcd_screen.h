#ifndef LCD_SCREEN_H
#define LCD_SCREEN_H

#include <Arduino.h>
#include "sensors.h"
#include "battery.h"

bool setupDisplay();
void showData(const SensorData& s, const BatteryData& b, const String& time);
void showMessage(const String& line1, const String& line2 = "", const String& line3 = "");
void showBootScreen();
void turnOffDisplay();
bool isDisplayReady();

#endif
