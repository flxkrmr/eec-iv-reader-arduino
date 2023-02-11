#ifndef EEC_IV_H
#define EEC_IV_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Cart.h"


class EecIv {
  public:
  typedef void (*callback_t)(const char []);
  typedef void (*callback_empty_t)(void);

  callback_t debugPrint;
  Cart::callback_t onFaultCodeFinished;
  callback_t onKoeoReadCode;
  callback_empty_t onKoeoFinished;
  callback_empty_t onStartMessageTimeout;
  
  enum OperationMode {
    READ_FAULTS,
    KOEO,
    LIVE_DATA
  } mode = READ_FAULTS;

  EecIv(int di, int ro, int re);

  void setMode(EecIv::OperationMode mode);
  void restartReading();

  void mainLoop();

  private:

  Cart* cart;

  enum State {
    IDLE,
    SEND_START_MESSAGE,
    CHANGE_BAUD_RATE_9600,
    WAIT_FOR_SYNC_9600,
  
    REQUEST_BAUD_RATE_CHANGE,
    WAIT_REQUEST_BAUD_RATE_CHANGE_DONE,
    CHANGE_BAUD_RATE_2400,
    WAIT_FOR_SYNC_2400,

    REQUEST_CLEAR_DCL_ERRORS,
    WAIT_REQUEST_CLEAR_DCL_ERRORS,

    REQUEST_CONT_SELF_TEST_CODES,
    WAIT_REQUEST_CONT_SELF_TEST_CODES,

    ENABLE_READING_SLOW_SYNC,
    ANSWER_SLOW_SYNC,

    ANSWER_REQUEST_FAULT_CODE,
    ANSWER_REQUEST_FAULT_CODE_SHORT,
    READ_REQUEST_FAULT_CODE,

    ANSWER_REQUEST_KOEO,
    ANSWER_REQUEST_KOEO_SHORT,
    WAIT_REQUEST_KOEO_SHORT,
    READ_REQUEST_KOEO,
    READ_REQUEST_KOEO_AFTER_ANSWER,

    ANSWER_REQUEST_LIVE_DATA,
    ANSWER_REQUEST_LIVE_DATA_SHORT,
    ANSWER_REQUEST_LIVE_DATA_INIT_SHIT
  } currentState = IDLE; 

  uint8_t syncPointer = 0;
  uint8_t errorCodePointer = 0;
  uint8_t loopCounter = 0;
  uint8_t koeoCounter = 0;
  unsigned long timeoutTimer = 0L;
  const unsigned long timeoutMax = 2000UL;
  uint8_t startMessageCounter = 0;
  const uint8_t startMessageCounterMax = 5;


  int waitSyncLoop();
  int waitSyncLoopShort();
  int answerFastSyncLoop();
  int answerSlowSyncLoop();

  int answerRequestFaultCode();
  int answerRequestFaultCodeShort();
  int readRequestFaultCode();
  
  int answerRequestKoeo();
  int answerRequestKoeoShort();
  int waitByte();
  int readRequestKoeo();

  int answerRequestLiveData();
  int answerRequestLiveDataShort();

  int exceededTimeout();
  void initTimeoutTimer();

  unsigned char errorCodeBuffer[2];

  char printBuffer[90];

  uint8_t buffer[4];
  void resetBuffer();
  void pushBuffer(uint8_t val);
  int pushAvailableToBuffer();
  bool isBufferSync(uint8_t syncPointer);

  const static uint8_t startSig[18];
};

#endif /* EEC_IV_H */
