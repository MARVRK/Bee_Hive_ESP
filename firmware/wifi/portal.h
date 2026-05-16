#ifndef PORTAL_H
#define PORTAL_H

#include <Arduino.h>

// Запускает AP + captive portal для настройки WiFi
// Возвращает true если пользователь сохранил новые credentials
bool startConfigPortal();

#endif
