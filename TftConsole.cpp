#include "TftConsole.h"

TftConsole::TftConsole(int cs, int dc, int rst) {
  tft = new TFT(cs, dc, rst);
}

void TftConsole::setup() {
  tft->begin();
  tft->background(0, 0, 0);
  tft->stroke(255, 255, 255);
  tft->setTextSize(1);
}

void TftConsole::printHexValue(int value) {
  nextLineIfLineIsFull();
  nextPageIfPageIsFull();
  
  sprintf(hexBuffer, "%02X ", value);
  tft->text(hexBuffer, signCounter*signWidth, lineCounter * lineHeight);
  signCounter+=3;
}

void TftConsole::printLine(char text[]) {
  nextLineIfLineIsStarted();
  nextPageIfPageIsFull();
  tft->text(text, 0, lineCounter * lineHeight);
  lineCounter++;
  signCounter = 0;
}

void TftConsole::nextPageIfPageIsFull() {
  if (lineCounter >= maxLine) {
    lineCounter = 0;
    tft->background(0,0,0);
  }
}

void TftConsole::nextLineIfLineIsFull() {
  if (signCounter >= maxSign) {
    signCounter = 0;
    lineCounter++;
  }
}

void TftConsole::nextLineIfLineIsStarted() {
  if (signCounter != 0) {
    lineCounter++;
    signCounter = 0;
  }
}
