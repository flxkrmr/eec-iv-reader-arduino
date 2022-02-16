#ifndef RS_485_H
#define RS_485_H

#include "arduino.h"
#include <SoftwareSerial.h>


class Rs485 {
  public:
  Rs485(int de, int re);
  void setup();

  int mainLoop();

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
    READ_REQUEST
  };

  State currentState = SEND_START_MESSAGE;

  int pin_de;
  int pin_re;

  char out_buffer[64];

  int syncPointer = 0;
  int loopCounter = 0;

  void sendStartMessage();
  int waitSyncLoop();
  int waitSyncLoopShort();
  int answerFastSyncLoop();
  int answerSlowSyncLoop();
  
  void rxMode(int baudrate);
  void enableWriteMode();
  void enableReadMode();

  SoftwareSerial *softwareSerial;
};

#endif /* RS_485 */
