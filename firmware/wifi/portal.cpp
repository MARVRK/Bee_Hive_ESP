#include "portal.h"
#include "wifi_module.h"
#include "../common/config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

static WebServer server(80);
static DNSServer dns;
static bool _credentialsSaved = false;

// ── HTML страница настройки ───────────────────────────────────────
static const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>BeeHive Setup</title>
  <style>
    body { font-family: sans-serif; max-width: 360px; margin: 40px auto; padding: 0 16px; }
    h2 { color: #f5a623; }
    input { width: 100%; padding: 10px; margin: 8px 0; box-sizing: border-box;
            border: 1px solid #ccc; border-radius: 6px; font-size: 16px; }
    button { width: 100%; padding: 12px; background: #f5a623; color: white;
             border: none; border-radius: 6px; font-size: 16px; cursor: pointer; }
    .status { margin-top: 16px; color: #555; font-size: 14px; }
  </style>
</head>
<body>
  <h2>🐝 BeeHive WiFi Setup</h2>
  <p>Введи данные домашней сети:</p>
  <form method="POST" action="/save">
    <input type="text"     name="ssid" placeholder="Имя WiFi сети (SSID)" required>
    <input type="password" name="pass" placeholder="Пароль" required>
    <button type="submit">Сохранить и перезапустить</button>
  </form>
  <p class="status">После сохранения устройство перезагрузится и подключится к сети.</p>
</body>
</html>
)rawliteral";

static const char HTML_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="utf-8"><title>Сохранено</title></head>
<body style="font-family:sans-serif;text-align:center;margin-top:60px;">
  <h2>✅ Сохранено!</h2>
  <p>BeeHive перезагружается и подключается к сети...</p>
</body>
</html>
)rawliteral";

// ── ОБРАБОТЧИКИ ──────────────────────────────────────────────────
static void handleRoot() {
    server.send(200, "text/html", HTML_PAGE);
}

static void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        String ssid = server.arg("ssid");
        String pass = server.arg("pass");
        ssid.trim();
        pass.trim();
        if (ssid.length() > 0) {
            saveWiFiCredentials(ssid, pass);
            server.send(200, "text/html", HTML_SAVED);
            _credentialsSaved = true;
            Serial.println("[PORTAL] Credentials сохранены: " + ssid);
            return;
        }
    }
    server.send(400, "text/plain", "Missing SSID");
}

static void handleNotFound() {
    // Captive portal: любой запрос → главная страница
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
}

// ── ОСНОВНАЯ ФУНКЦИЯ ─────────────────────────────────────────────
bool startConfigPortal() {
    Serial.println("[PORTAL] Запуск AP: " PORTAL_SSID);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(PORTAL_SSID); // без пароля — открытая сеть
    Serial.printf("[PORTAL] IP: %s\n", WiFi.softAPIP().toString().c_str());

    // DNS сервер — все запросы → 192.168.4.1 (captive portal)
    dns.start(53, "*", WiFi.softAPIP());

    server.on("/",     HTTP_GET,  handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.onNotFound(handleNotFound);
    server.begin();

    _credentialsSaved = false;
    unsigned long lastActivity = millis();

    Serial.printf("[PORTAL] Ожидание %d сек...\n", PORTAL_TIMEOUT / 1000);

    while (!_credentialsSaved && millis() - lastActivity < PORTAL_TIMEOUT) {
        dns.processNextRequest();
        server.handleClient();
        delay(10);

        // Сбрасываем таймер если есть клиент
        if (WiFi.softAPgetStationNum() > 0) {
            lastActivity = millis();
        }
    }

    server.stop();
    dns.stop();
    WiFi.softAPdisconnect(true);

    if (_credentialsSaved) {
        Serial.println("[PORTAL] Настройка завершена — перезагружаемся");
        delay(500);
        ESP.restart();
    } else {
        Serial.println("[PORTAL] Таймаут — возврат к основной логике");
    }

    return _credentialsSaved;
}
