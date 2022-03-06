#include "EecIv.h"
#include "TftConsole.h"
#include <EasyButton.h>

// Pins Display
static const int CS=10;
static const int DC=12;
static const int RST=0;

// Pins RS485
static const int DI=3;
static const int RE=6;
static const int RO=2;

// Pins Buttons
static const int BTN_1 = 7;
static const int BTN_2 = 8;
static const int BTN_3 = 0;

void serialPrint(char message[]) {
  Serial.println(message);
}

EecIv eecIv = EecIv(DI, RO, RE, serialPrint);

EasyButton button1(BTN_1);
EasyButton button2(BTN_2);

//TftConsole tftConsole(CS, DC, RST);

int mode = 0;

void setup() {
  Serial.begin(19200);
  Serial.println("### EEC IV Reader ###");

  button1.begin();
  button1.onPressed(restartButtonCallback);
  button2.begin();
  button2.onPressed(modeButtonCallback);

  eecIv.setup();

  eecIv.setModeFaultCode();
}

void loop() {
  eecIv.mainLoop();
  button1.read();
  button2.read();
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
