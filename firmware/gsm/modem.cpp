// A7670E modem control
#include <Arduino.h>

void setupModem() {
    // TODO:
    // - Serial2 init
    // - PWRKEY sequence
    // - AT commands init
    // - GPRS connect
}

void powerOnModem() {
    // TODO: PWRKEY sequence
}

void powerOffModem() {
    // TODO: power down
}

bool connectGPRS() {
    // TODO: APN connect
    return false;
}

int getSignalStrength() {
    // TODO: AT+CSQ
    return 0;
}
