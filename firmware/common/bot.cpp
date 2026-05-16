#include "bot.h"

// ── /status ──────────────────────────────────────────────────────
String formatStatus(const SensorData& s, const BatteryData& b, const String& time) {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "🐝 <b>Bee Hive Status</b>\n"
        "🕐 %s\n\n"
        "⚖️ Вес:       <b>%.3f кг</b>\n"
        "🌡 Темп:      <b>%.1f °C</b>\n"
        "💧 Влажн:     <b>%.0f %%</b>\n"
        "🔋 Батарея:   <b>%d %% (%.2f В)</b>\n"
        "⏱ Остаток:   <b>~%d ч</b>",
        time.c_str(),
        s.weight,
        s.temperature,
        s.humidity,
        b.percent, b.voltage,
        b.estimatedHours
    );
    return String(buf);
}

// ── /telemetry ───────────────────────────────────────────────────
String formatTelemetry(const SensorData& s, const BatteryData& b,
                       float lat, float lon, bool locFresh,
                       int signalDbm, const String& connType,
                       int failures, const String& time) {
    char buf[512];
    snprintf(buf, sizeof(buf),
        "📊 <b>Telemetry</b>\n"
        "🕐 %s\n\n"
        "⚖️  weight_kg:   <code>%.3f</code>\n"
        "🌡  temp_c:      <code>%.1f</code>\n"
        "💧  humidity:    <code>%.0f</code>\n"
        "🔋  bat_voltage: <code>%.2f V</code>\n"
        "📶  bat_pct:     <code>%d %%</code>\n"
        "⏱   bat_hours:  <code>%d</code>\n"
        "📡  signal:      <code>%d dBm</code>\n"
        "🌐  conn:        <code>%s</code>\n"
        "📍  location:    <code>%.6f, %.6f</code>%s\n"
        "❌  failures:    <code>%d</code>",
        time.c_str(),
        s.weight, s.temperature, s.humidity,
        b.voltage, b.percent, b.estimatedHours,
        signalDbm,
        connType.c_str(),
        lat, lon, locFresh ? "" : " (cached)",
        failures
    );
    return String(buf);
}

// ── /location ────────────────────────────────────────────────────
String formatLocation(float lat, float lon, bool fresh) {
    if (lat == 0 && lon == 0) {
        return "📍 Координаты ещё не получены";
    }
    char buf[200];
    snprintf(buf, sizeof(buf),
        "📍 <b>Местоположение</b>\n"
        "Lat: <code>%.6f</code>\n"
        "Lon: <code>%.6f</code>\n"
        "<a href=\"https://maps.google.com/?q=%.6f,%.6f\">Открыть в картах</a>\n"
        "%s",
        lat, lon, lat, lon,
        fresh ? "" : "⚠️ Данные из кэша (GPS не обновлялся)"
    );
    return String(buf);
}

// ── /schedule show ───────────────────────────────────────────────
String formatScheduleInfo(int intervalSec) {
    char buf[128];
    if (intervalSec >= 3600) {
        snprintf(buf, sizeof(buf), "⏰ Расписание: каждые %d ч", intervalSec / 3600);
    } else {
        snprintf(buf, sizeof(buf), "⏰ Расписание: каждые %d мин", intervalSec / 60);
    }
    return String(buf);
}
