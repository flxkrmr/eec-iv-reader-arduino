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
int mode = 0;

void setup() {
  Serial.begin(19200);

  eecIv.print = &serialPrint;
  eecIv.setModeKoeo();
  eecIv.setup();
}

void loop() {
  eecIv.mainLoop();
}

void restartButtonCallback() {
  eecIv.restartReading();
}

void modeButtonCallback() {
  mode++;
  if (mode > 2) 
    mode = 0;

  switch(mode) {
    case 0:
      eecIv.setModeFaultCode();
      break;
    case 1:
      eecIv.setModeKoeo();
      break;
    case 2:
      eecIv.setModeLiveData();
      break;
  }
}

void serialPrint(char message[]) {
  Serial.println(message);
}