#ifndef EEC_IV_H
#define EEC_IV_H

#include "arduino.h"
#include <SoftwareSerial.h>
#include "EecIvCommon.h"
#include "FaultCode.h"

class EecIv : EecIvCommon {
  public:
  EecIv(int di, int ro, int re, EecIvCommon::callback_t printCallback);

  void setup();

  void setModeFaultCode();
  void setModeKoeo();
  void setModeLiveData();

  void restartReading();

  int mainLoop();

  
  enum OperationMode {
    READ_FAULTS,
    KOEO,
    LIVE_DATA
  };

  private:

  enum State {
    IDLE,
    SEND_START_MESSAGE,
    ENABLE_READING_FAST_SYNC,
    WAIT_FAST_SYNC,
    ANSWER_FAST_SYNC,
    WAIT_FAST_SYNC_SHORT,
    ENABLE_READING_SLOW_SYNC,
    ANSWER_SLOW_SYNC,

    FAULT_CODE,

    ANSWER_REQUEST_KOEO,
    ANSWER_REQUEST_KOEO_SHORT,
    WAIT_REQUEST_KOEO_SHORT,
    READ_REQUEST_KOEO,
    READ_REQUEST_KOEO_AFTER_ANSWER,

    ANSWER_REQUEST_LIVE_DATA,
    ANSWER_REQUEST_LIVE_DATA_SHORT,
    ANSWER_REQUEST_LIVE_DATA_INIT_SHIT
  };

  State currentState = IDLE; 
  OperationMode mode = READ_FAULTS;
  SoftwareSerial *softwareSerial;
  callback_t print;

  FaultCode *faultCodeReader;

  int pin_re;

  int syncPointer = 0;
  int errorCodePointer = 0;
  int errorCodeCounter = 0;
  int loopCounter = 0;
  int koeoCounter = 0;
  unsigned long timeoutTimer = 0L;
  const unsigned long timeoutMax = 5000UL;

  char out_buf[90];

  void sendStartMessage();

  int waitSyncLoop();
  int waitSyncLoopShort();
  int answerFastSyncLoop();
  int answerSlowSyncLoop();
  
  int answerRequestKoeo();
  int answerRequestKoeoShort();
  int waitByte();
  int readRequestKoeo();

  int answerRequestLiveData();
  int answerRequestLiveDataShort();
  int answerRequestLiveDataInitShit();

  int exceededTimeout();
  void initTimeoutTimer();

  void answer(unsigned char message[], int delay);

  
  void rxMode(int baudrate);
  void enableWriteMode();
  void enableReadMode();

  unsigned char errorCodeBuffer[2];

  char printBuffer[90];

  unsigned char buffer[4];
  void pushBuffer(unsigned char val);
  void initBuffer();
  int pushAvailableToBuffer();


  const static unsigned char syncSig[4][4];
  const static unsigned char startSig[18];
};

#endif /* EEC_IV_H */
