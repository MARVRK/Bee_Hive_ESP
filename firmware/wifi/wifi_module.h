#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <Arduino.h>

bool   setupWiFi();
bool   connectWiFi(int timeoutMs = 15000);
void   disconnectWiFi();
bool   isWiFiConnected();
int    getWiFiSignalDbm();

// Сохранить/загрузить учётные данные из NVS
void   saveWiFiCredentials(const String& ssid, const String& password);
String loadWiFiSSID();
String loadWiFiPassword();
bool   hasWiFiCredentials();

#endif
