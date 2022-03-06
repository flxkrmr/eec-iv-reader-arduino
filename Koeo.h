#ifndef KOEO_H
#define KOEO_H

#include "arduino.h"
#include <SoftwareSerial.h>
#include "EecIvCommon.h"

class Koeo : EecIvCommon {
  public:
  Koeo(SoftwareSerial *softwareSerial, int re, EecIvCommon::callback_t printCallback);

  int mainLoop();

  private:
  callback_t print;

  enum State {
    ANSWER_REQUEST_KOEO,
    ANSWER_REQUEST_KOEO_SHORT,
    WAIT_REQUEST_KOEO_SHORT,
    READ_REQUEST_KOEO,
    READ_REQUEST_KOEO_AFTER_ANSWER,
  };
  
  State currentState = ANSWER_REQUEST_KOEO;   
  
  int loopCounter = 0;
  int koeoCounter = 0;
  int syncPointer = 0;

  int answerRequestKoeo();
  int answerRequestKoeoShort();
  int waitByte();
  int readRequestKoeo();

  unsigned char buffer[4];
  void pushBuffer(unsigned char val);
  void initBuffer(unsigned char buffer[]);
  int pushAvailableToBuffer();
  
  const static unsigned char syncSig[4][4];
};

#endif /* KOEO_H */