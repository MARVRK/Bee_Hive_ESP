#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>
#include "sensors.h"
#include "battery.h"

struct LogEntry {
    String    timestamp;   // ISO8601 из RTC или millis
    SensorData sensors;
    BatteryData battery;
    float     lat;
    float     lon;
    bool      locationFresh;
    int       signalDbm;
    String    connType;    // "WiFi" или "GSM"
    int       sendFailures;
};

bool setupSD();
bool logToSD(const LogEntry& entry);
bool flushBuffer();          // отправить накопленные неотправленные записи
int  getBufferedCount();
void pruneOldLogs();         // удалить файлы старше SD_MAX_DAYS дней
bool isSDReady();

#endif
