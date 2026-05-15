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
