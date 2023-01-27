#include "VoltageReader.h"

VoltageReader::VoltageReader(uint8_t voltagePin) {
    this->voltagePin = voltagePin;
}

void VoltageReader::loop() {
    long now = millis();
    if (now - lastMeasurement > INTERVAL_MILLIS) {
        lastMeasurement = now;

        int value = analogRead(voltagePin);
        
        double voltage = value * (27.405 / 1023.0);
        onVoltage(voltage);
    }
} 