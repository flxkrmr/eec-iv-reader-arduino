#ifndef FAULT_CODE_H
#define FAULT_CODE_H

#include "arduino.h"
#include <SoftwareSerial.h>
#include "EecIvCommon.h"

class FaultCode : EecIvCommon {
  public:
  FaultCode(SoftwareSerial *softwareSerial, int re, EecIvCommon::callback_t printCallback);

  int mainLoop();

  private:
  SoftwareSerial *softwareSerial;
  callback_t print;

  enum State {
    ANSWER_REQUEST_FAULT_CODE,
    ANSWER_REQUEST_FAULT_CODE_SHORT,
    READ_REQUEST_FAULT_CODE
  };
  
  State currentState = ANSWER_REQUEST_FAULT_CODE;   
  
  int pin_re;

  int syncPointer = 0;
  int errorCodePointer = 0;
  const static unsigned char syncSig[4][4];

  char printBuffer[90];

  int answerRequestFaultCode();
  int answerRequestFaultCodeShort();
  int readRequestFaultCode();
  
  unsigned char buffer[4];
  void pushBuffer(unsigned char val);
  void initBuffer();
  int pushAvailableToBuffer();

  void answer(unsigned char message[], int delay);
 
  void rxMode(int baudrate);
  void enableWriteMode();
  void enableReadMode();
};

#endif