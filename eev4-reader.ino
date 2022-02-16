#include "Rs485.h"

// Pins Display
static const int CS=10;
static const int DC=9;
static const int RST=0;

// Pins RS485
static const int DI=1;
static const int DE=6;
static const int RE=7;
static const int RO=0;

Rs485 rs485 = Rs485(DE, RE);

void setup() {
  rs485.setup();
}

void loop() {
  rs485.mainLoop();
}
