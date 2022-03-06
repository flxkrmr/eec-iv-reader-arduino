#ifndef EEC_IV_H
#define EEC_IV_H

#include "arduino.h"
#include <SoftwareSerial.h>
#include "EecIvCommon.h"
#include "FaultCode.h"
#include "Koeo.h"
#include "LiveData.h"

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
    RUN_KOEO,
    READ_LIVE_DATA
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
    KOEO,
    LIVE_DATA
  };

  State currentState = IDLE; 
  OperationMode mode = READ_FAULTS;
  callback_t print;

  FaultCode *faultCodeReader;
  Koeo *koeoReader;
  LiveData *liveDataReader;

  int syncPointer = 0;
  int loopCounter = 0;
  unsigned long timeoutTimer = 0L;
  const unsigned long timeoutMax = 5000UL;

  char out_buf[90];

  void sendStartMessage();

  int waitSyncLoop();
  int waitSyncLoopShort();
  int answerFastSyncLoop();
  int answerSlowSyncLoop();

  int exceededTimeout();
  void initTimeoutTimer();

  char printBuffer[90];

  unsigned char buffer[4];
  void pushBuffer(unsigned char val);
  void initBuffer();
  int pushAvailableToBuffer();

  const static unsigned char startSig[18];
};

#endif /* EEC_IV_H */
