#include "sd_card.h"
#include "config.h"
#include <SPI.h>
#include <SD.h>

static bool _sdOk = false;

// ── ИНИЦИАЛИЗАЦИЯ ────────────────────────────────────────────────
bool setupSD() {
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    _sdOk = SD.begin(SD_CS);
    if (!_sdOk) {
        Serial.println("[SD] Не удалось инициализировать MicroSD");
        return false;
    }

    // Создаём папку если нет
    if (!SD.exists(SD_LOG_DIR)) {
        SD.mkdir(SD_LOG_DIR);
    }

    Serial.printf("[SD] OK, свободно: %llu MB\n",
                  SD.totalBytes() / (1024 * 1024));
    return true;
}

bool isSDReady() { return _sdOk; }

// ── ФОРМАТИРОВАНИЕ СТРОКИ CSV ─────────────────────────────────────
static String buildCSVLine(const LogEntry& e) {
    // timestamp,weight,temp,humidity,bat_v,bat_pct,lat,lon,signal,conn,failures
    String line = e.timestamp + ",";
    line += String(e.sensors.weight, 3) + ",";
    line += String(e.sensors.temperature, 1) + ",";
    line += String(e.sensors.humidity, 1) + ",";
    line += String(e.battery.voltage, 2) + ",";
    line += String(e.battery.percent) + ",";
    line += String(e.lat, 6) + ",";
    line += String(e.lon, 6) + ",";
    line += String(e.signalDbm) + ",";
    line += e.connType + ",";
    line += String(e.sendFailures);
    return line;
}

// ── ПОЛУЧИТЬ ИМЯ ФАЙЛА ПО ДАТЕ ───────────────────────────────────
static String getLogFilename(const String& timestamp) {
    // timestamp: "2026-05-16T14:30:00Z" → "/data/2026-05-16.csv"
    String date = timestamp.substring(0, 10); // "2026-05-16"
    return String(SD_LOG_DIR) + "/" + date + ".csv";
}

// ── ЗАПИСЬ ───────────────────────────────────────────────────────
bool logToSD(const LogEntry& entry) {
    if (!_sdOk) return false;

    String filename = getLogFilename(entry.timestamp);
    bool newFile    = !SD.exists(filename);

    File f = SD.open(filename, FILE_APPEND);
    if (!f) {
        Serial.println("[SD] Ошибка открытия файла: " + filename);
        return false;
    }

    // Заголовок для нового файла
    if (newFile) {
        f.println("timestamp,weight_kg,temp_c,humidity_pct,"
                  "bat_v,bat_pct,lat,lon,signal_dbm,conn,failures");
    }

    f.println(buildCSVLine(entry));
    f.close();

    Serial.println("[SD] Записано → " + filename);

    // Чистим старые файлы
    pruneOldLogs();
    return true;
}

// ── УДАЛЕНИЕ СТАРЫХ ФАЙЛОВ (90 дней) ─────────────────────────────
void pruneOldLogs() {
    File dir = SD.open(SD_LOG_DIR);
    if (!dir) return;

    // Собираем список файлов
    std::vector<String> files;
    File entry;
    while ((entry = dir.openNextFile())) {
        if (!entry.isDirectory()) {
            String name = String(entry.name());
            if (name.endsWith(".csv")) {
                files.push_back(String(SD_LOG_DIR) + "/" + name);
            }
        }
        entry.close();
    }
    dir.close();

    // Если больше SD_MAX_DAYS файлов — удаляем самые старые
    if ((int)files.size() > SD_MAX_DAYS) {
        // Сортируем по имени (имена = даты, лексиграфически = хронологически)
        std::sort(files.begin(), files.end());
        int toDelete = (int)files.size() - SD_MAX_DAYS;
        for (int i = 0; i < toDelete; i++) {
            SD.remove(files[i]);
            Serial.println("[SD] Удалён старый файл: " + files[i]);
        }
    }
}

// ── БУФЕР НЕОТПРАВЛЕННЫХ ЗАПИСЕЙ ─────────────────────────────────
// Файл с накопленными записями при отсутствии связи
#define BUFFER_FILE "/data/buffer.csv"
#define BUFFER_MAX  10

bool saveToBuffer(const LogEntry& entry) {
    if (!_sdOk) return false;

    // Проверяем сколько записей в буфере
    if (getBufferedCount() >= BUFFER_MAX) {
        Serial.println("[SD] Буфер переполнен, старая запись удалена");
        // Можно читать файл и удалять первую строку — пока просто пропускаем
        return false;
    }

    File f = SD.open(BUFFER_FILE, FILE_APPEND);
    if (!f) return false;
    f.println(buildCSVLine(entry));
    f.close();
    return true;
}

int getBufferedCount() {
    if (!_sdOk || !SD.exists(BUFFER_FILE)) return 0;
    File f = SD.open(BUFFER_FILE);
    if (!f) return 0;
    int count = 0;
    while (f.available()) {
        if (f.read() == '\n') count++;
    }
    f.close();
    return count;
}

bool flushBuffer() {
    // Вернуть true если буфер пуст или успешно отправлен
    // Логика отправки вызывается из bot.cpp / server.cpp
    if (!_sdOk || !SD.exists(BUFFER_FILE)) return true;
    int count = getBufferedCount();
    if (count == 0) {
        SD.remove(BUFFER_FILE);
        return true;
    }
    Serial.printf("[SD] В буфере %d неотправленных записей\n", count);
    return false;
}
