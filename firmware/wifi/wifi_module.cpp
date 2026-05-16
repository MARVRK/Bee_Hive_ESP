#include "wifi_module.h"
#include "../common/config.h"
#include "secrets.h"
#include <WiFi.h>
#include <Preferences.h>

static Preferences prefs;
static const char* NVS_NS = "wifi";

// ── ИНИЦИАЛИЗАЦИЯ ────────────────────────────────────────────────
bool setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);
    Serial.println("[WiFi] Инициализирован (STA режим)");
    return true;
}

// ── ПОДКЛЮЧЕНИЕ ──────────────────────────────────────────────────
bool connectWiFi(int timeoutMs) {
    String ssid = loadWiFiSSID();
    String pass = loadWiFiPassword();

    // Fallback на secrets.h если NVS пустой
    if (ssid.isEmpty()) {
        ssid = WIFI_SSID_DEFAULT;
        pass = WIFI_PASS_DEFAULT;
    }

    if (ssid.isEmpty()) {
        Serial.println("[WiFi] SSID не задан. Нужна настройка через портал");
        return false;
    }

    Serial.printf("[WiFi] Подключение к '%s'...\n", ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < (unsigned long)timeoutMs) {
        delay(200);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Подключено! IP: %s  Signal: %d dBm\n",
                      WiFi.localIP().toString().c_str(),
                      WiFi.RSSI());
        return true;
    }

    Serial.println("[WiFi] Не удалось подключиться");
    return false;
}

// ── ОТКЛЮЧЕНИЕ ───────────────────────────────────────────────────
void disconnectWiFi() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("[WiFi] Отключено");
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

int getWiFiSignalDbm() {
    return WiFi.RSSI();
}

// ── NVS: сохранение учётных данных ──────────────────────────────
void saveWiFiCredentials(const String& ssid, const String& password) {
    prefs.begin(NVS_NS, false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();
    Serial.printf("[WiFi] Credentials сохранены: '%s'\n", ssid.c_str());
}

String loadWiFiSSID() {
    prefs.begin(NVS_NS, true);
    String s = prefs.getString("ssid", "");
    prefs.end();
    return s;
}

String loadWiFiPassword() {
    prefs.begin(NVS_NS, true);
    String p = prefs.getString("pass", "");
    prefs.end();
    return p;
}

bool hasWiFiCredentials() {
    return !loadWiFiSSID().isEmpty();
}
