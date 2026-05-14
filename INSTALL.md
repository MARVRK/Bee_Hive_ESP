# 📦 Установка файлов проекта Bee Hive ESP

## Структура проекта

```
BeeHive/V2/dev/
├── src/
│   └── main.cpp              ← Основной код
├── test/
│   └── test_peripherals.cpp  ← Тесты периферии
├── include/
│   └── secrets.h.example     ← Шаблон для секретов
├── docs/
│   ├── WIRING.txt           ← Схема подключения
│   ├── QUICK_START.md       ← Быстрый старт
│   └── GIT_WORKFLOW.md      ← Git руководство
├── .gitignore               ← Игнорируемые файлы
├── platformio.ini           ← Конфигурация PlatformIO
├── Makefile                 ← Быстрые команды
└── README.md                ← Описание проекта
```

## 🚀 Способ 1: Автоматически (скрипт)

Скопируйте все файлы из архива в вашу папку проекта:

```bash
cd /d/NextCloud/3D_NAS_Code/BeeHive/V2/dev

# Создайте структуру папок
mkdir -p src test include docs

# ВАЖНО: Файлы нужно распаковать из архива и разложить по папкам!
```

## 📝 Способ 2: Вручную (Windows)

1. **Откройте папку проекта**
   ```
   D:\NextCloud\3D_NAS_Code\BeeHive\V2\dev\
   ```

2. **Создайте папки:**
   - Правой кнопкой → Создать → Папка
   - Создайте: `src`, `test`, `include`, `docs`

3. **Разложите файлы:**

   **В корень проекта:**
   - `platformio.ini`
   - `Makefile`
   - `README.md`
   - `.gitignore`

   **В папку src/:**
   - `main.cpp`

   **В папку test/:**
   - `test_peripherals.cpp`

   **В папку include/:**
   - `secrets.h.example`

   **В папку docs/:**
   - `WIRING.txt`
   - `QUICK_START.md`
   - `GIT_WORKFLOW.md`

## ⚡ После копирования

### 1️⃣ Создайте secrets.h
```bash
cd include
cp secrets.h.example secrets.h
# Откройте secrets.h и заполните свои данные
```

### 2️⃣ Добавьте в Git
```bash
cd /d/NextCloud/3D_NAS_Code/BeeHive/V2/dev

git add .
git commit -m "Initial commit: Bee Hive monitoring system"
git push -u origin main
```

### 3️⃣ Откройте проект в VS Code
```bash
code .
```

Или через VS Code:
- File → Open Folder
- Выберите: `D:\NextCloud\3D_NAS_Code\BeeHive\V2\dev`

### 4️⃣ Проверьте что PlatformIO видит проект
- В VS Code внизу должна появиться панель PlatformIO
- Если нет - перезапустите VS Code

## 🧪 Тестирование

### Запуск тестов периферии:
```bash
# Через Makefile
make test

# Или через PlatformIO
pio run -e test -t upload
pio device monitor
```

### Основная программа:
```bash
make upload monitor

# Или
pio run -t upload
pio device monitor
```

## 📋 Проверка структуры

В VS Code должна быть видна такая структура:

```
DEV
├── .pio/                 (появится после первой компиляции)
├── .vscode/              (автоматически от PlatformIO)
├── docs/
│   ├── GIT_WORKFLOW.md
│   ├── QUICK_START.md
│   └── WIRING.txt
├── include/
│   ├── secrets.h         (создайте из .example)
│   └── secrets.h.example
├── src/
│   └── main.cpp
├── test/
│   └── test_peripherals.cpp
├── .gitignore
├── Makefile
├── platformio.ini
└── README.md
```

## ❓ Проблемы?

### "PlatformIO не видит проект"
- Убедитесь что `platformio.ini` в корне
- Перезапустите VS Code
- Откройте папку проекта, а не родительскую

### "secrets.h не найден"
```bash
cd include
cp secrets.h.example secrets.h
```

### "Git ругается на secrets.h"
Это нормально! `secrets.h` в `.gitignore` и не должен коммититься

## ✅ Готово!

После правильной установки можно:
```bash
make push MSG='feat: initial setup'  # Закоммитить
make upload monitor                  # Загрузить на ESP32
```

🐝 Удачи с проектом!
