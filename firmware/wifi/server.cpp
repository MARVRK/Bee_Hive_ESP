#include "server.h"
#include "../common/bot.h"
#include "../common/config.h"
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

static long _lastUpdateId = 0;
static Preferences _prefs;

// ── NVS: расписание и настройки ──────────────────────────────────
void saveSchedule(int intervalSec) {
    _prefs.begin("schedule", false);
    _prefs.putInt("interval", intervalSec);
    _prefs.end();
}

int loadSchedule() {
    _prefs.begin("schedule", true);
    int v = _prefs.getInt("interval", SLEEP_TIME_SEC);
    _prefs.end();
    return v;
}

void saveNotifEnabled(bool enabled) {
    _prefs.begin("schedule", false);
    _prefs.putBool("notif", enabled);
    _prefs.end();
}

bool loadNotifEnabled() {
    _prefs.begin("schedule", true);
    bool v = _prefs.getBool("notif", true);
    _prefs.end();
    return v;
}

// ── ОТПРАВКА СООБЩЕНИЯ В TELEGRAM ────────────────────────────────
bool sendTelegramMessage(const String& text) {
    WiFiClientSecure client;
    client.setInsecure(); // без верификации CA (приемлемо для IoT)

    HTTPClient http;
    String url = String(TELEGRAM_API) + "/bot" + TELEGRAM_BOT_TOKEN + "/sendMessage";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    // Формируем JSON тело
    StaticJsonDocument<1024> doc;
    doc["chat_id"]    = TELEGRAM_CHAT_ID;
    doc["text"]       = text;
    doc["parse_mode"] = "HTML";

    String body;
    serializeJson(doc, body);

    int code = http.POST(body);
    http.end();

    if (code == 200) {
        Serial.println("[TG] Сообщение отправлено");
        return true;
    }
    Serial.printf("[TG] Ошибка отправки: HTTP %d\n", code);
    return false;
}

// ── ОТПРАВКА ТЕЛЕМЕТРИИ ──────────────────────────────────────────
bool sendTelemetryNow(const SensorData& s, const BatteryData& b,
                      float lat, float lon, bool locFresh,
                      const String& time) {
    int signal = WiFi.RSSI();
    String msg = formatTelemetry(s, b, lat, lon, locFresh,
                                 signal, "WiFi", 0, time);
    return sendTelegramMessage(msg);
}

// ── LONG POLLING: ОЖИДАНИЕ И ОБРАБОТКА КОМАНД ────────────────────
void pollTelegramCommands(const SensorData& s, const BatteryData& b,
                          float lat, float lon, bool locFresh,
                          const String& time, int intervalSec) {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = String(TELEGRAM_API) + "/bot" + TELEGRAM_BOT_TOKEN
                 + "/getUpdates?offset=" + String(_lastUpdateId + 1)
                 + "&timeout=" + String(TELEGRAM_TIMEOUT);

    http.begin(client, url);
    http.setTimeout((TELEGRAM_TIMEOUT + 5) * 1000);

    Serial.printf("[TG] Long polling %d сек...\n", TELEGRAM_TIMEOUT);
    int code = http.GET();

    if (code != 200) {
        Serial.printf("[TG] getUpdates ошибка: HTTP %d\n", code);
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();

    // Парсим JSON
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, payload) != DeserializationError::Ok) {
        Serial.println("[TG] JSON parse error");
        return;
    }

    JsonArray results = doc["result"].as<JsonArray>();
    for (JsonObject upd : results) {
        long id = upd["update_id"].as<long>();
        if (id > _lastUpdateId) _lastUpdateId = id;

        String text    = upd["message"]["text"].as<String>();
        String chatId  = upd["message"]["chat"]["id"].as<String>();
        text.trim();

        // Проверяем что команда от нашего chat_id
        if (chatId != String(TELEGRAM_CHAT_ID)) continue;

        Serial.println("[TG] Команда: " + text);

        // ── ОБРАБОТКА КОМАНД ──────────────────────────────────────
        if (text == "/status") {
            sendTelegramMessage(formatStatus(s, b, time));

        } else if (text == "/telemetry") {
            sendTelemetryNow(s, b, lat, lon, locFresh, time);

        } else if (text == "/location") {
            sendTelegramMessage(formatLocation(lat, lon, locFresh));

        } else if (text == "/schedule show") {
            sendTelegramMessage(formatScheduleInfo(intervalSec));

        } else if (text.startsWith("/schedule every ")) {
            // /schedule every 30m  или  /schedule every 1h
            String arg = text.substring(16);
            int newInterval = 0;
            if (arg.endsWith("h")) {
                newInterval = arg.toInt() * 3600;
            } else if (arg.endsWith("m")) {
                newInterval = arg.toInt() * 60;
            }
            if (newInterval >= SLEEP_MIN_SEC) {
                saveSchedule(newInterval);
                // Обновляем текущий интервал (применится со следующего цикла)
                intervalSec = newInterval;
                sendTelegramMessage("✅ " + formatScheduleInfo(newInterval)
                                    + "\n(применится с следующего цикла)");
            } else {
                sendTelegramMessage("❌ Минимальный интервал — 15 минут\n"
                                    "Примеры: /schedule every 1h  /schedule every 30m");
            }

        } else if (text == "/notifications on") {
            saveNotifEnabled(true);
            sendTelegramMessage("🔔 Уведомления включены");

        } else if (text == "/notifications off") {
            saveNotifEnabled(false);
            sendTelegramMessage("🔕 Уведомления выключены");

        } else if (text == "/start" || text == "/help") {
            sendTelegramMessage(
                "🐝 <b>BeeHive Bot</b>\n\n"
                "/status — состояние улья\n"
                "/telemetry — полная телеметрия\n"
                "/location — координаты\n"
                "/schedule show — текущее расписание\n"
                "/schedule every 1h — каждый час\n"
                "/schedule every 30m — каждые 30 мин\n"
                "/notifications on|off — уведомления"
            );
        } else {
            sendTelegramMessage("❓ Неизвестная команда. /help для справки");
        }
    }
}
