#include "Rs485.h"

// Pins Display
static const int CS=10;
static const int DC=9;
static const int RST=0;

// Pins RS485
static const int DI=3;
static const int RE=6;
static const int RO=2;

Rs485 rs485 = Rs485(DI, RO, RE);


void setup() {
  Serial.begin(19200);

  rs485.print = &serialPrint;
  rs485.setup();
}

void loop() {
  rs485.mainLoop();
}

void serialPrint(char message[]) {
  Serial.println(message);
}