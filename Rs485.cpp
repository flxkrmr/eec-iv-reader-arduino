#include "Rs485.h"

const unsigned char Rs485::syncSig[4][4] = {
    {0x00, 0x00, 0x00, 0xa0 },
    {0x00, 0x00, 0x00, 0xb1 },
    {0x00, 0x00, 0x00, 0x82 },
    {0x00, 0x00, 0x00, 0x93 }
  };

const unsigned char Rs485::startSig[18] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x00, 0x00,
    0x00, 0x05
  };

Rs485::Rs485(int di, int ro, int re) {
  pin_re = re;

  clearBuffer();
  softwareSerial = new SoftwareSerial(di, ro);
}

void Rs485::setup() {
  pinMode(pin_re,OUTPUT);
}

void Rs485::gotoSlowSync() {
  currentState = ENABLE_READING_SLOW_SYNC;
}

void Rs485::answer(unsigned char message[], int delay) {
  softwareSerial->write(message[0]);
  delayMicroseconds(delay);
  softwareSerial->write(message[1]);
}

void Rs485::sendStartMessage() {  
  softwareSerial->begin(2400);  
  enableWriteMode();

  for(int i = 0; i<sizeof(startSig); i++) {
    softwareSerial->write(startSig[i]); // using softwareSerial as it can be timed better!
    delayMicroseconds(420); // try and error. Off delay has to be ~850 us
  }
}

void Rs485::rxMode(int baudrate) {
  softwareSerial->begin(baudrate);
  enableReadMode();
}

void Rs485::enableWriteMode() {
  digitalWrite(pin_re, HIGH);
}

void Rs485::enableReadMode() {
  digitalWrite(pin_re, LOW);
}


int Rs485::mainLoop() {
  switch(currentState) {
    case IDLE:
      break;
    case SEND_START_MESSAGE:
      sendStartMessage();
      print("Send start message");
      currentState = ENABLE_READING_FAST_SYNC;
      break;

    case ENABLE_READING_FAST_SYNC:
      rxMode(9600);
      initTimeoutTimer();
      currentState = WAIT_FAST_SYNC;
    case WAIT_FAST_SYNC:
      if (waitSyncLoop()) {
        currentState = ANSWER_FAST_SYNC;
      } else {
        if(exceededTimeout()) {
          sprintf(out_buf, "Exceeded timeout fast sync %lu, %lu, %lu", millis(), timeoutTimer, timeoutMax);
          print(out_buf);
          currentState = SEND_START_MESSAGE;
        }
      }
      break;

    case ANSWER_FAST_SYNC:
      if (answerFastSyncLoop()) {
        currentState = ENABLE_READING_SLOW_SYNC;
      }
      break;
    case ENABLE_READING_SLOW_SYNC:
      rxMode(2400);
      initTimeoutTimer();
      currentState = ANSWER_SLOW_SYNC;
    case ANSWER_SLOW_SYNC:
      if(answerSlowSyncLoop()) {
        loopCounter++;
        if(loopCounter >= 2) {
          loopCounter = 0;
          currentState = ANSWER_REQUEST;
        }
      } else {
        if(exceededTimeout()) {
          sprintf(out_buf, "Exceeded timeout slow sync %lu, %lu, %lu", millis(), timeoutTimer, timeoutMax);
          print(out_buf);
          currentState = SEND_START_MESSAGE;
        }
      }
      break;
    case ANSWER_REQUEST:
      if (answerRequest()) {
        currentState = ANSWER_REQUEST_SHORT;
      }
      break;
    case ANSWER_REQUEST_SHORT:
      if (answerRequestShort()) {
        currentState = READ_REQUEST;
      }
      break;
    case READ_REQUEST:
      if (readRequest()) {
        errorCodeCounter++;
        if (errorCodeCounter > 4) {
          currentState = IDLE;
        } else {
          currentState = ANSWER_REQUEST;
        }
      }
      break;
  }
}

int Rs485::exceededTimeout() {
  if (millis() > timeoutTimer + timeoutMax) {
    return 1;
  }
  return 0;
}

void Rs485::initTimeoutTimer() {
  timeoutTimer = millis();
}

int Rs485::waitSyncLoop() {
  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {      
      syncPointer++;
      clearBuffer();

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int Rs485::waitSyncLoopShort() {
  unsigned char syncSig[4] = {0x00, 0x00, 0x00, 0xa0 }; 

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig, buffer, 4)) {    
      return 1;
    }
  }

  return 0;
}

int Rs485::answerFastSyncLoop() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x81, 0x74 },
    {0x00, 0xa0 }
  };
  
  
  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(426);
      enableWriteMode();
      answer(answerSig[syncPointer], 15);
      enableReadMode(); 
      
      syncPointer++;
      clearBuffer();

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int Rs485::answerSlowSyncLoop() {

  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x01, 0xF4 },
    {0x00, 0xa0 }
  };  

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      enableWriteMode();
      answer(answerSig[syncPointer], 60);
      enableReadMode(); 
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int Rs485::answerRequest() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x26, 0xa4 },
    {0x00, 0xa0 }
  };  

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      enableWriteMode();
      answer(answerSig[syncPointer], 60);
      enableReadMode(); 
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int Rs485::answerRequestShort() {
  unsigned char answerSig[2] = {0x01, 0xb0 };

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      enableWriteMode();
      answer(answerSig, 60);
      enableReadMode(); 
      
      syncPointer++;
      return 1;
    }
  }

  return 0;
}

int Rs485::readRequest() { 

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    errorCodePointer++;

    if(errorCodePointer >= 2) {
      errorCodePointer = 0;

      // TODO put error code into error set
      sprintf(printBuffer, "Found Error Code: %01X%02X", buffer[3] & 0xF, buffer[2]);
      print(printBuffer);
      return 1;
    }
  }

  return 0;
}


void Rs485::pushBuffer(unsigned char val) {
  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = val;
}

void Rs485::clearBuffer() {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = 0;
  }
}