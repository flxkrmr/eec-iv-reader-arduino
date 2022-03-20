#ifndef LIVE_DATA_H
#define LIVE_DATA_H

#include "arduino.h"
#include <SoftwareSerial.h>
#include "EecIvCommon.h"

class LiveData : EecIvCommon {
  public:
  LiveData(SoftwareSerial *softwareSerial, int re, EecIvCommon::callback_t printCallback);

  int mainLoop();

  private:
  callback_t print;
  
  int syncPointer = 0;
  unsigned char buffer[4];
  void pushBuffer(unsigned char val);
  void initBuffer();
  int pushAvailableToBuffer();

  enum State {
    REQUEST_LIVE_DATA,
    INIT_LIVE_DATA_MESSAGE_1,
    INIT_LIVE_DATA_MESSAGE_2,
    INIT_LIVE_DATA_MESSAGE_3,
    INIT_LIVE_DATA_MESSAGE_4
  };

  State currentState = REQUEST_LIVE_DATA; 

  int requestLiveData();
  int initLiveDataMessage1();
  int initLiveDataMessage2();
  int initLiveDataMessage3();
  int initLiveDataMessage4();

};

#endif /* LIVE_DATA_H */