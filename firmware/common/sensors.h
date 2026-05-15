#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

void setupSensors();
void readSensors();
float getTemperature();
float getHumidity();
float getWeight();

#endif
