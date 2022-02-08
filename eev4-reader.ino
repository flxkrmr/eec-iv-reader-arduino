#include "TftConsole.h"
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


TftConsole tftConsole = TftConsole(CS, DC, RST);
Rs485 rs485 = Rs485(DE, RE);

void setup() {
  tftConsole.setup();
  rs485.setup();

  tftConsole.printLine("Sending Start Message");
  rs485.txStartMessage();
  rs485.rxMode9600();

}

int val = 0;
void loop() {
  if (rs485.syncLoop(val) > 0) {
    tftConsole.printLine("Found Sync");
    val = 1;
  }

/*
  if (rs485.available() > 0) {
    val = rs485.read();
    tftConsole.printHexValue(val);
  }
  */
}
