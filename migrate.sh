#!/bin/bash
# Bee Hive ESP - Automated Project Restructure Script
# Миграция на модульную архитектуру (GSM + WiFi версии)

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
print_header() {
    echo -e "\n${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Check if we're in the right directory
check_directory() {
    print_header "Проверка директории"
    
    if [ ! -f "platformio.ini" ]; then
        print_error "Ошибка: platformio.ini не найден!"
        print_error "Запустите скрипт из корня проекта BeeHive/V2/dev/"
        exit 1
    fi
    
    if [ ! -d ".git" ]; then
        print_error "Ошибка: .git не найден!"
        print_error "Это не Git репозиторий!"
        exit 1
    fi
    
    print_success "Директория OK: $(pwd)"
    
    # Check current branch
    current_branch=$(git branch --show-current)
    if [ "$current_branch" = "testing" ]; then
        print_warning "Вы на ветке 'testing' с рабочими тестами!"
        echo ""
        echo "Рекомендую:"
        echo "  1. Переключиться на main: git checkout main"
        echo "  2. Затем запустить скрипт снова"
        echo ""
        echo "Ветка 'testing' останется нетронутой!"
        echo ""
        read -p "Переключить на main автоматически? (y/n): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            git checkout main 2>/dev/null || git checkout master 2>/dev/null || {
                print_error "Не удалось переключиться на main/master"
                exit 1
            }
            print_success "Переключено на $(git branch --show-current)"
        else
            print_warning "Миграция отменена. Переключитесь на нужную ветку вручную."
            exit 0
        fi
    fi
}

# Show current state
show_current_state() {
    print_header "Текущее состояние проекта"
    
    echo "Ветка: $(git branch --show-current)"
    echo "Коммитов: $(git rev-list --count HEAD)"
    echo ""
    echo "Структура:"
    tree -L 2 -I '.pio|.git' || ls -la
}

# Confirmation prompt
confirm_migration() {
    print_header "Подтверждение миграции"
    
    echo -e "${YELLOW}Этот скрипт выполнит:${NC}"
    echo "  1. Создаст backup (тег v0.1-tests + ветка backup)"
    echo "  2. Создаст структуру firmware/{common,gsm,wifi}"
    echo "  3. Переместит файлы в новую структуру"
    echo "  4. Создаст шаблоны модулей"
    echo "  5. Обновит platformio.ini и Makefile"
    echo "  6. Создаст ветки development, feature/gsm, feature/wifi"
    echo ""
    
    read -p "Продолжить? (y/n): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_warning "Миграция отменена"
        exit 0
    fi
}

# Create backup
create_backup() {
    print_header "Создание backup"
    
    # Commit current state
    git add . 2>/dev/null || true
    git commit -m "checkpoint: state before restructure" 2>/dev/null || true
    
    # Create tag
    git tag v0.1-tests 2>/dev/null || print_warning "Тег v0.1-tests уже существует"
    
    # Create backup branch
    git branch backup-before-restructure 2>/dev/null || print_warning "Ветка backup уже существует"
    
    print_success "Backup создан: тег v0.1-tests, ветка backup-before-restructure"
}

# Create new structure
create_structure() {
    print_header "Создание структуры папок"
    
    mkdir -p firmware/common
    mkdir -p firmware/gsm
    mkdir -p firmware/wifi
    mkdir -p docs
    mkdir -p hardware
    
    print_success "Структура создана"
}

# Move existing files
move_files() {
    print_header "Перемещение файлов"
    
    # Backup current src/main.cpp to firmware/gsm/
    if [ -f "src/main.cpp" ]; then
        cp src/main.cpp firmware/gsm/main_old.cpp
        print_success "src/main.cpp → firmware/gsm/main_old.cpp"
    fi
    
    # Keep test files as-is
    if [ -d "test" ]; then
        print_success "test/ сохранен без изменений"
    fi
    
    # Move docs
    if [ -f "WIRING.txt" ]; then
        mv WIRING.txt docs/ 2>/dev/null || true
        print_success "WIRING.txt → docs/"
    fi
}

# Create module templates
create_modules() {
    print_header "Создание шаблонов модулей"
    
    # Common modules
    cat > firmware/common/sensors.h << 'EOF'
#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

void setupSensors();
void readSensors();
float getTemperature();
float getHumidity();
float getWeight();

#endif
EOF

    cat > firmware/common/sensors.cpp << 'EOF'
#include "sensors.h"
#include <Adafruit_SHT31.h>
#include <HX711.h>

// TODO: Implement sensor functions
void setupSensors() {
    // Initialize SHT30, HX711
}

void readSensors() {
    // Read all sensors
}

float getTemperature() { return 0.0; }
float getHumidity() { return 0.0; }
float getWeight() { return 0.0; }
EOF

    cat > firmware/common/display.h << 'EOF'
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void setupDisplay();
void updateDisplay();
void turnOffDisplay();

#endif
EOF

    cat > firmware/common/display.cpp << 'EOF'
#include "display.h"
#include <Adafruit_SSD1306.h>

// TODO: Implement display functions
void setupDisplay() {}
void updateDisplay() {}
void turnOffDisplay() {}
EOF

    cat > firmware/common/config.h << 'EOF'
#ifndef CONFIG_H
#define CONFIG_H

// GPIO Configuration
#define I2C_SDA 21
#define I2C_SCL 22
#define HX711_DOUT 25
#define HX711_SCK 26
#define SD_CS 5
#define BUTTON_PIN 13
#define BATTERY_ADC 34

// Timing
#define SLEEP_TIME 3600  // 1 hour

#endif
EOF

    # GSM specific
    cat > firmware/gsm/main.cpp << 'EOF'
/*
 * Bee Hive Monitor - GSM Version
 */

#include <Arduino.h>
#include "../common/config.h"
#include "../common/sensors.h"
#include "../common/display.h"

void setup() {
    Serial.begin(115200);
    Serial.println("🐝 Bee Hive - GSM Version");
    
    // TODO: Initialize GSM module
}

void loop() {
    // TODO: Main loop
}
EOF

    # WiFi specific
    cat > firmware/wifi/main.cpp << 'EOF'
/*
 * Bee Hive Monitor - WiFi Version
 */

#include <Arduino.h>
#include "../common/config.h"
#include "../common/sensors.h"
#include "../common/display.h"

void setup() {
    Serial.begin(115200);
    Serial.println("🐝 Bee Hive - WiFi Version");
    
    // TODO: Initialize WiFi
}

void loop() {
    // TODO: Main loop
}
EOF

    print_success "Модули созданы"
}

# Create new platformio.ini
create_platformio_ini() {
    print_header "Создание platformio.ini"
    
    cat > platformio.ini << 'EOF'
[platformio]
default_envs = gsm

; ===== COMMON SETTINGS =====
[env]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600

; ===== GSM VERSION =====
[env:gsm]
build_src_filter = 
    +<../firmware/common/*>
    +<../firmware/gsm/*>
    -<../firmware/wifi/*>
    -<../test/*>

lib_deps = 
    adafruit/Adafruit SHT31 Library@^2.2.2
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.3
    bogde/HX711@^0.7.5
    vshymanskyy/TinyGSM@^0.12.0
    bblanchon/ArduinoJson@^7.2.1
    adafruit/RTClib@^2.1.1

build_flags = 
    -DVERSION_GSM
    -DCORE_DEBUG_LEVEL=3

; ===== WIFI VERSION =====
[env:wifi]
build_src_filter = 
    +<../firmware/common/*>
    +<../firmware/wifi/*>
    -<../firmware/gsm/*>
    -<../test/*>

lib_deps = 
    adafruit/Adafruit SHT31 Library@^2.2.2
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.3
    bogde/HX711@^0.7.5
    bblanchon/ArduinoJson@^7.2.1
    adafruit/RTClib@^2.1.1
    ESP Async WebServer@^1.2.3

build_flags = 
    -DVERSION_WIFI
    -DCORE_DEBUG_LEVEL=3

; ===== TEST ENVIRONMENT =====
[env:test]
build_src_filter = +<../test/*>

lib_deps = 
    adafruit/Adafruit SHT31 Library@^2.2.2
    bogde/HX711@^0.7.5

build_flags = -DCORE_DEBUG_LEVEL=3
EOF

    print_success "platformio.ini создан"
}

# Create new Makefile
create_makefile() {
    print_header "Создание Makefile"
    
    cat > Makefile << 'EOF'
.PHONY: help gsm wifi test clean

help:
	@echo "🐝 Bee Hive ESP - Commands:"
	@echo ""
	@echo "  make gsm-build      - Build GSM version"
	@echo "  make gsm-upload     - Upload GSM version"
	@echo "  make wifi-build     - Build WiFi version"
	@echo "  make wifi-upload    - Upload WiFi version"
	@echo "  make test           - Upload test firmware"
	@echo "  make clean          - Clean build files"

gsm-build:
	pio run -e gsm

gsm-upload:
	pio run -e gsm -t upload

gsm: gsm-upload
	pio device monitor

wifi-build:
	pio run -e wifi

wifi-upload:
	pio run -e wifi -t upload

wifi: wifi-upload
	pio device monitor

test:
	pio run -e test -t upload
	pio device monitor

clean:
	pio run -t clean

monitor:
	pio device monitor
EOF

    print_success "Makefile создан"
}

# Create documentation
create_docs() {
    print_header "Создание документации"
    
    cat > docs/TZ.md << 'EOF'
# Техническое задание
# Система мониторинга пасечного улья

## Версия: 2.0
## Дата: 2026-05-15

См. полное ТЗ в прикрепленном документе.

## Архитектура

### Модульная структура:
- `firmware/common/` - общий код для обеих версий
- `firmware/gsm/` - GSM-специфичный код
- `firmware/wifi/` - WiFi-специфичный код

### Две версии прошивки:
1. GSM версия (A7670E + Telegram)
2. WiFi версия (WiFi + Web interface)
EOF

    cat > README.md << 'EOF'
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
EOF

    print_success "Документация создана"
}

# Git operations
git_operations() {
    print_header "Git операции"
    
    # Stage all changes
    git add .
    
    # Commit
    git commit -m "refactor: modular architecture for GSM/WiFi versions

- Created firmware/{common,gsm,wifi} structure
- Separated PlatformIO environments
- Added module templates
- Preserved test code
- Based on TZ v2.0 requirements

BREAKING CHANGE: Project structure completely reorganized"
    
    print_success "Изменения закоммичены"
    
    # Create branches
    git checkout -b development 2>/dev/null || git checkout development
    print_success "Ветка development создана"
    
    git checkout -b feature/gsm-implementation 2>/dev/null || git checkout feature/gsm-implementation
    print_success "Ветка feature/gsm-implementation создана"
    
    git checkout development
    git checkout -b feature/wifi-implementation 2>/dev/null || git checkout feature/wifi-implementation
    print_success "Ветка feature/wifi-implementation создана"
    
    git checkout development
}

# Final summary
show_summary() {
    print_header "✅ МИГРАЦИЯ ЗАВЕРШЕНА!"
    
    echo -e "${GREEN}Что сделано:${NC}"
    echo "  ✓ Backup: тег v0.1-tests, ветка backup-before-restructure"
    echo "  ✓ Структура: firmware/{common,gsm,wifi}"
    echo "  ✓ Модули: sensors, display, config шаблоны"
    echo "  ✓ platformio.ini: 3 environment (gsm, wifi, test)"
    echo "  ✓ Makefile: команды для сборки"
    echo "  ✓ Документация: README.md, docs/TZ.md"
    echo "  ✓ Ветки: development, feature/gsm, feature/wifi"
    echo "  ✓ Ветка 'testing' СОХРАНЕНА с рабочими тестами!"
    echo ""
    
    echo -e "${BLUE}Доступные ветки:${NC}"
    git branch --list
    echo ""
    
    echo -e "${BLUE}Текущая ветка:${NC} $(git branch --show-current)"
    echo ""
    
    echo -e "${YELLOW}Следующие шаги:${NC}"
    echo "  1. Проверь структуру: ls -la firmware/"
    echo "  2. Собери GSM версию: make gsm-build"
    echo "  3. Начни разработку: git checkout feature/gsm-implementation"
    echo "  4. Push на GitHub: git push -u origin development"
    echo ""
    echo "  Тесты: git checkout testing (СОХРАНЕНЫ!)"
    echo ""
    
    echo -e "${GREEN}Откат (если нужно):${NC}"
    echo "  git checkout backup-before-restructure"
    echo "  или"
    echo "  git checkout testing  # вернуться к тестам"
    echo ""
}

# Main execution
main() {
    clear
    echo -e "${BLUE}"
    echo "╔═══════════════════════════════════════════╗"
    echo "║   🐝 BEE HIVE ESP - AUTO MIGRATION      ║"
    echo "║   Модульная архитектура GSM/WiFi        ║"
    echo "╚═══════════════════════════════════════════╝"
    echo -e "${NC}\n"
    
    check_directory
    show_current_state
    confirm_migration
    
    create_backup
    create_structure
    move_files
    create_modules
    create_platformio_ini
    create_makefile
    create_docs
    git_operations
    
    show_summary
}

# Run
main
