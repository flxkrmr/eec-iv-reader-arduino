#ifndef RS_485_H
#define RS_485_H

#include "arduino.h"
#include <SoftwareSerial.h>


class Rs485 {
  public:
  Rs485(int di, int ro, int re);

  void setup();
  int mainLoop();

  void (*print)(char[]);

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
    ANSWER_REQUEST,
    ANSWER_REQUEST_SHORT,
    READ_REQUEST
  };

  State currentState = ENABLE_READING_SLOW_SYNC;

  int pin_re;

  int syncPointer = 0;
  int errorCodePointer = 0;
  int errorCodeCounter = 0;
  int loopCounter = 0;
  unsigned long timeoutTimer = 0L;
  const unsigned long timeoutMax = 5000UL;

  char out_buf[90];

  void sendStartMessage();
  int waitSyncLoop();
  int waitSyncLoopShort();
  int answerFastSyncLoop();
  int answerSlowSyncLoop();
  int answerRequest();
  int answerRequestShort();
  int readRequest();

  int exceededTimeout();
  void initTimeoutTimer();

  void answer(unsigned char message[], int delay);

  void gotoSlowSync();

  
  void rxMode(int baudrate);
  void enableWriteMode();
  void enableReadMode();

  unsigned char errorCodeBuffer[2];

  char printBuffer[90];

  unsigned char buffer[4];
  void pushBuffer(unsigned char val);
  void clearBuffer();

  const static unsigned char syncSig[4][4];
  const static unsigned char startSig[18];

  SoftwareSerial *softwareSerial;
};

#endif /* RS_485 */
