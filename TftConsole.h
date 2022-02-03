#ifndef TFT_CONSOLE_H
#define TFT_CONSOLE_H

#include <TFT.h> 
#include <SPI.h>

class TftConsole {
  private:
  int lineCounter = 0;
  int signCounter = 0;
  int lineHeight = 10;
  int signWidth = 5;
  
  int maxLine = 13;
  int maxSign = 26;

  char hexBuffer[6];
  TFT *tft;

  public:
  TftConsole(int cs, int dc, int rst);
  void setup();
  void printHexValue(int value);  
  void printLine(char text[]);

  private:
  void nextPageIfPageIsFull();
  void nextLineIfLineIsFull();
  void nextLineIfLineIsStarted();
};

#endif /* TFT_CONSOLE_H */
