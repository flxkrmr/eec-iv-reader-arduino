#ifndef EEC_IV_H
#define EEC_IV_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "Cart.h"


class EecIv {
  public:
  typedef void (*callback_int_t)(const uint8_t []);
  typedef void (*callback_t)(const char []);
  typedef void (*callback_empty_t)(void);

  callback_t debugPrint;
  callback_int_t onFaultCodeRead;
  callback_empty_t onFaultCodeFinished;
  callback_empty_t onStartMessageTimeout;

  callback_int_t onLiveData;
  
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
    CHECK_IF_IN_DIAG_MODE,
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
    READ_CONT_SELF_TEST_CODES,

    REQUEST_KOEO,
    WAIT_REQUEST_KOEO,
    READ_KOEO,

    REQUEST_PID_MODE,
    WAIT_PID_MODE,
    TRANSMIT_PID_MAP,
    WAIT_TRANSMIT_PID_MAP,
    WAIT_PID_DATA,
  
  } currentState = IDLE; 

  uint8_t syncPointer = 0;
  uint8_t errorCodePointer = 0;
  uint8_t loopCounter = 0;
  uint8_t koeoCounter = 0;
  unsigned long timeoutTimer = 0L;
  const unsigned long timeoutMax = 3000UL;
  uint8_t startMessageCounter = 0;
  const uint8_t startMessageCounterMax = 5;

  int exceededTimeout();
  void initTimeoutTimer();

  unsigned char errorCodeBuffer[2];

  char printBuffer[90];

  uint8_t liveDataBuf[32];
  uint8_t liveDataOffset = 0;
  uint8_t liveDataLastFrame = 0;

  uint8_t buffer[4];
  void resetBuffer();
  void pushBuffer(uint8_t val);
  int pushAvailableToBuffer();
  bool isBufferSync(uint8_t syncPointer);
};

#endif /* EEC_IV_H */
