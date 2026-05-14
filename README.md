# 🏋️ IoT Weight & Climate Monitor

ESP32-based IoT device for weight monitoring with climate sensors and 4G LTE connectivity.

[![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## 📋 Features

- ⚖️ **Weight measurement** with 4x 50kg load cells + HX711 ADC
- 🌡️ **Temperature & humidity** monitoring via SHT30 sensor
- 📡 **4G LTE connectivity** using A7670E-FASE module
- 💾 **Local data logging** on MicroSD card
- 📱 **Telegram bot** integration for remote monitoring
- 🔋 **Battery powered** with voltage monitoring
- 💤 **Deep sleep mode** for extended battery life

## 🛠️ Hardware

### Core Components
- **MCU**: ESP32-WROOM-32D (dual-core, WiFi/BT)
- **Connectivity**: A7670E-FASE (4G LTE + GPS)
- **Sensors**:
  - 4x 50kg load cells with HX711 24-bit ADC
  - SHT30 temperature/humidity sensor (I2C)
- **Storage**: MicroSD card module (SPI)
- **Power**: 
  - HW-465A BMS + boost module (1S2P 18650 config)
  - Voltage monitoring via ADC

### GPIO Mapping
```
ESP32 Pin   →   Component
─────────────────────────
IO4         →   A7670E PWRKEY
IO16        →   A7670E RX
IO17        →   A7670E TX
IO21        →   SHT30 SDA
IO22        →   SHT30 SCL
IO25        →   HX711 DOUT
IO26        →   HX711 SCK
IO18        →   SD SCK
IO19        →   SD MISO
IO23        →   SD MOSI
IO5         →   SD CS
IO34        →   Battery voltage (ADC)
```

## 🚀 Quick Start

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- ESP32 board
- Hardware components assembled

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/iot-weight-monitor.git
   cd iot-weight-monitor
   ```

2. **Configure secrets**
   ```bash
   cp secrets.h.example secrets.h
   # Edit secrets.h with your credentials
   ```

3. **Build and upload**
   ```bash
   make upload monitor
   # or
   pio run -t upload && pio device monitor
   ```

### Using Makefile

```bash
make help                           # Show all commands
make push MSG='feat: new feature'   # Quick git commit + push
make build                          # Compile code
make upload                         # Flash to ESP32
make monitor                        # Open Serial Monitor
```

## 📊 Project Structure

```
iot-weight-monitor/
├── src/
│   └── main.cpp              # Main application code
├── test/
│   └── peripheral_test.cpp   # Hardware test suite
├── docs/
│   ├── WIRING.txt           # Connection diagrams
│   └── CALIBRATION.md       # Sensor calibration guide
├── platformio.ini            # PlatformIO configuration
├── secrets.h.example         # Template for credentials
├── .gitignore               # Git ignore rules
├── Makefile                 # Quick commands
└── README.md                # This file
```

## 🔧 Configuration

### WiFi (optional, for setup)
```cpp
#define WIFI_SSID "your_ssid"
#define WIFI_PASSWORD "your_password"
```

### 4G LTE
```cpp
#define GPRS_APN "internet"  // Your carrier APN
#define GPRS_USER ""
#define GPRS_PASSWORD ""
```

### Telegram Bot
```cpp
#define TELEGRAM_BOT_TOKEN "123456:ABC-DEF..."
#define TELEGRAM_CHAT_ID "your_chat_id"
```

## 📡 Telegram Commands

- `/start` - Start monitoring
- `/stop` - Stop monitoring  
- `/status` - Current readings
- `/tare` - Zero the scale
- `/calibrate` - Calibration mode

## 🔋 Power Consumption

- **Active mode**: ~200mA @ 3.7V
- **Deep sleep**: ~10mA @ 3.7V
- **Battery life**: ~14 days with hourly wakeups (1S2P 7Ah pack)

## ⚠️ Important Notes

### A7670E Module
- **CRITICAL**: 470µF electrolytic capacitor required on VBAT pin
- Without capacitor, module will reset during transmission (2A peaks)
- Micro USB port is only for firmware updates, not needed for operation

### HX711 Calibration
1. Power on without load
2. Run tare procedure
3. Place known weight
4. Calculate calibration factor
5. Update `HX711_CALIBRATION_FACTOR` in secrets.h

## 📚 Documentation

- [Hardware Setup](docs/WIRING.txt)
- [Sensor Calibration](docs/CALIBRATION.md)
- [Telegram Bot Setup](docs/TELEGRAM.md)
- [Power Management](docs/POWER.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

## 🐛 Debugging

### Test Peripherals
```bash
# Upload test suite
pio run -e test -t upload
pio device monitor
```

### Check Serial Output
```bash
make monitor
# or
pio device monitor -b 115200
```

### Common Issues
- **SHT30 not found**: Check I2C address (0x44 or 0x45), try swapping SDA/SCL
- **A7670E no response**: Verify 470µF capacitor, check power supply (4V/2A)
- **SD card failure**: Ensure FAT32 format, check SPI connections
- **HX711 unstable**: Add 100nF capacitors near sensors, shield cables

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'feat: add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [TinyGSM](https://github.com/vshymanskyy/TinyGSM) - GSM/LTE modem library
- [Adafruit SHT31](https://github.com/adafruit/Adafruit_SHT31) - SHT30/31 sensor library
- [HX711](https://github.com/bogde/HX711) - Load cell amplifier library

Made with ❤️ and ☕ by Rosty
