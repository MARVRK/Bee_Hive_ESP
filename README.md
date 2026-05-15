# 🐝 Bee Hive ESP32 Monitor

IoT система мониторинга пасечного улья

## 📦 Две версии прошивки:

### GSM версия (основная)
- A7670E 4G LTE + GPS
- Telegram бот
- Автономная работа

### WiFi версия (альтернативная)
- WiFi подключение
- Web interface
- Настройка через AP

## 🚀 Быстрый старт:

```bash
# GSM версия
make gsm-upload

# WiFi версия
make wifi-upload

# Тесты
make test
```

## 📁 Структура:

```
firmware/
├── common/     # Общий код (sensors, display, etc.)
├── gsm/        # GSM версия
└── wifi/       # WiFi версия
```

## 🔧 Разработка:

- `feature/gsm` - разработка GSM версии
- `feature/wifi` - разработка WiFi версии
- `development` - интеграция

## 📚 Документация:

См. `docs/TZ.md`
