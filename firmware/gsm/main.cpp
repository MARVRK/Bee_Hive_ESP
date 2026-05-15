// GSM version main
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    Serial.println("BEE HIVE - GSM");
    
    // TODO:
    // - setupSensors()
    // - setupSD()
    // - setupBattery()
    // - setupModem()
    // - readSensors()
    // - logToSD()
    // - sendTelegram()
    // - getGPS() (once per day)
    // - goToSleep()
}

void loop() {
    // Empty - using deep sleep
}
