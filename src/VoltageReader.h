#pragma once

#include "Arduino.h"

#define INTERVAL_MILLIS 1000

class VoltageReader {
    public:
    typedef void (*callback_t)(const double);

    VoltageReader(uint8_t voltagePin);

    callback_t onVoltage;
    void loop();


    private:
    long lastMeasurement;
    uint8_t voltagePin;

};