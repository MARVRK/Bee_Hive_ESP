#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include "../common/sensors.h"
#include "../common/battery.h"

// Отправка данных и обработка команд Telegram через WiFi
bool sendTelegramMessage(const String& text);
bool sendTelemetryNow(const SensorData& s, const BatteryData& b,
                      float lat, float lon, bool locFresh,
                      const String& time);
void pollTelegramCommands(const SensorData& s, const BatteryData& b,
                          float lat, float lon, bool locFresh,
                          const String& time, int& intervalSec);

// NVS: расписание и уведомления
void saveSchedule(int intervalSec);
int  loadSchedule();
void saveNotifEnabled(bool enabled);
bool loadNotifEnabled();

#endif
