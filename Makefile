# Makefile для быстрых команд проекта IoT Weight Monitor

.PHONY: help push commit build upload monitor clean status

# Default target
help:
	@echo "🚀 IoT Weight Monitor - Быстрые команды"
	@echo ""
	@echo "Git команды:"
	@echo "  make push MSG='описание'  - add + commit + push"
	@echo "  make commit MSG='описание' - add + commit"
	@echo "  make status                - git status"
	@echo ""
	@echo "PlatformIO команды:"
	@echo "  make build                 - скомпилировать код"
	@echo "  make upload                - загрузить на ESP32"
	@echo "  make monitor               - открыть Serial Monitor"
	@echo "  make clean                 - очистить build файлы"
	@echo ""
	@echo "Примеры:"
	@echo "  make push MSG='feat: добавил тест SHT30'"
	@echo "  make upload monitor"
	@echo ""

# Git: add + commit + push одной командой
push:
ifndef MSG
	@echo "❌ Укажите сообщение: make push MSG='ваше сообщение'"
	@exit 1
endif
	@echo "📦 Добавляю файлы..."
	@git add .
	@echo "💾 Коммичу: $(MSG)"
	@git commit -m "$(MSG)"
	@echo "🚀 Отправляю на GitHub..."
	@git push
	@echo "✅ Готово!"

# Git: add + commit (без push)
commit:
ifndef MSG
	@echo "❌ Укажите сообщение: make commit MSG='ваше сообщение'"
	@exit 1
endif
	@echo "📦 Добавляю файлы..."
	@git add .
	@echo "💾 Коммичу: $(MSG)"
	@git commit -m "$(MSG)"
	@echo "✅ Закоммичено! (для отправки: git push)"

# Git: статус
status:
	@git status

# PlatformIO: сборка
build:
	@echo "🔨 Компилирую..."
	@pio run

# PlatformIO: загрузка на ESP32
upload:
	@echo "📤 Загружаю на ESP32..."
	@pio run -t upload

# PlatformIO: Serial Monitor
monitor:
	@echo "📺 Открываю Serial Monitor (Ctrl+C для выхода)..."
	@pio device monitor

# PlatformIO: очистка
clean:
	@echo "🗑️  Очищаю build файлы..."
	@pio run -t clean

# Быстрая разработка: build + upload + monitor
dev: build upload monitor

# Инициализация проекта
init:
	@echo "🔧 Инициализация проекта..."
	@if [ ! -f secrets.h ]; then \
		echo "📝 Создаю secrets.h из шаблона..."; \
		cp secrets.h.example secrets.h; \
		echo "⚠️  Не забудьте заполнить secrets.h своими данными!"; \
	fi
	@if [ ! -d .git ]; then \
		echo "🎯 Инициализирую Git..."; \
		git init; \
		git add .; \
		git commit -m "Initial commit"; \
	fi
	@echo "✅ Проект готов!"
