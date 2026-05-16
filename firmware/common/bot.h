#ifndef BOT_H
#define BOT_H

#include <Arduino.h>
#include "sensors.h"
#include "battery.h"

// Форматирование сообщений для Telegram
// Отправка выполняется в server.cpp (WiFi) или gsm/bot_gsm.cpp (GSM)

String formatStatus(const SensorData& s, const BatteryData& b, const String& time);
String formatTelemetry(const SensorData& s, const BatteryData& b,
                       float lat, float lon, bool locFresh,
                       int signalDbm, const String& connType,
                       int failures, const String& time);
String formatLocation(float lat, float lon, bool fresh);
String formatScheduleInfo(int intervalSec);

#endif
