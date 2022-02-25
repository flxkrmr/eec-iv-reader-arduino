#include "EecIv.h"

// Pins Display
static const int CS=10;
static const int DC=9;
static const int RST=0;

// Pins RS485
static const int DI=3;
static const int RE=6;
static const int RO=2;

EecIv eecIv = EecIv(DI, RO, RE);


void setup() {
  Serial.begin(19200);

  eecIv.print = &serialPrint;
  eecIv.setModeLiveData();
  eecIv.setup();
}

void loop() {
  eecIv.mainLoop();
}

void serialPrint(char message[]) {
  Serial.println(message);
}