#include "EecIv.h"

const unsigned char EecIv::syncSig[4][4] = {
    {0x00, 0x00, 0x00, 0xa0 },
    {0x00, 0x00, 0x00, 0xb1 },
    {0x00, 0x00, 0x00, 0x82 },
    {0x00, 0x00, 0x00, 0x93 }
  };

const unsigned char EecIv::startSig[18] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x00, 0x00,
    0x00, 0x05
  };

EecIv::EecIv(int di, int ro, int re) {
  pin_re = re;

  clearBuffer();
  softwareSerial = new SoftwareSerial(di, ro);
}

void EecIv::setup() {
  pinMode(pin_re,OUTPUT);
}

void EecIv::gotoSlowSync() {
  currentState = ENABLE_READING_SLOW_SYNC;
}

void EecIv::answer(unsigned char message[], int delay) {
  enableWriteMode();
  softwareSerial->write(message[0]);
  delayMicroseconds(delay);
  softwareSerial->write(message[1]);
  enableReadMode(); 
}

void EecIv::sendStartMessage() {  
  softwareSerial->begin(2400);  
  enableWriteMode();

  for(int i = 0; i<sizeof(startSig); i++) {
    softwareSerial->write(startSig[i]); // using softwareSerial as it can be timed better!
    delayMicroseconds(420); // try and error. Off delay has to be ~850 us
  }
}

void EecIv::rxMode(int baudrate) {
  softwareSerial->begin(baudrate);
  enableReadMode();
}

void EecIv::enableWriteMode() {
  digitalWrite(pin_re, HIGH);
}

void EecIv::enableReadMode() {
  digitalWrite(pin_re, LOW);
}

void EecIv::setModeFaultCode() {
  this->mode = READ_FAULTS;
}

void EecIv::setModeKoeo() {
  this->mode = KOEO;
}

void EecIv::setModeLiveData() {
  this->mode = LIVE_DATA;
}


int EecIv::mainLoop() {
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
          if (mode == READ_FAULTS) {
            currentState = ANSWER_REQUEST_FAULT_CODE;
          } else if (mode == KOEO) {
            currentState = ANSWER_REQUEST_KOEO;
          } else if (mode == LIVE_DATA) {
            currentState = ANSWER_REQUEST_LIVE_DATA;
          } else {
            currentState = IDLE;
          }
        }
      } else {
        if(exceededTimeout()) {
          sprintf(out_buf, "Exceeded timeout slow sync %lu, %lu, %lu", millis(), timeoutTimer, timeoutMax);
          print(out_buf);
          currentState = SEND_START_MESSAGE;
        }
      }
      break;

    case ANSWER_REQUEST_FAULT_CODE:
      if (answerRequestFaultCode()) {
        currentState = ANSWER_REQUEST_FAULT_CODE_SHORT;
      }
      break;
    case ANSWER_REQUEST_FAULT_CODE_SHORT:
      if (answerRequestFaultCodeShort()) {
        currentState = READ_REQUEST_FAULT_CODE;
      }
      break;
    case READ_REQUEST_FAULT_CODE:
      if (readRequestFaultCode()) {
        errorCodeCounter++;
        if (errorCodeCounter > 4) {
          currentState = IDLE;
        } else {
          currentState = ANSWER_REQUEST_FAULT_CODE;
        }
      }
      break;

    case ANSWER_REQUEST_KOEO:
      if (answerRequestKoeo()) {
        loopCounter++;
        if (loopCounter > 8) {
          currentState = IDLE;
          loopCounter = 0;
        }
      }
      break;

    case ANSWER_REQUEST_LIVE_DATA:
      answerRequestLiveData();
      break;
  }
}

int EecIv::exceededTimeout() {
  if (millis() > timeoutTimer + timeoutMax) {
    return 1;
  }
  return 0;
}

void EecIv::initTimeoutTimer() {
  timeoutTimer = millis();
}

int EecIv::pushAvailableToBuffer() {
  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());
    return 1;
  } else {
    return 0;
  }
}

int EecIv::waitSyncLoop() {
  if (pushAvailableToBuffer()) {
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

int EecIv::waitSyncLoopShort() {
  unsigned char syncSig[4] = {0x00, 0x00, 0x00, 0xa0 }; 

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig, buffer, 4)) {    
      return 1;
    }
  }

  return 0;
}

int EecIv::answerFastSyncLoop() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x81, 0x74 },
    {0x00, 0xa0 }
  };
  
  
  if (pushAvailableToBuffer()) {
    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(426);
      answer(answerSig[syncPointer], 15);
      
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

int EecIv::answerSlowSyncLoop() {

  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x01, 0xF4 },
    {0x00, 0xa0 }
  };  

  if (pushAvailableToBuffer()) {
    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[syncPointer], 60);
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}
int EecIv::answerRequestFaultCode() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x26, 0xa4 },
    {0x00, 0xa0 }
  };  

  if (pushAvailableToBuffer()) {
    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[syncPointer], 60);
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int EecIv::answerRequestKoeo() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x25, 0x94 },
    {0x00, 0xa0 }
  };  

  if (pushAvailableToBuffer()) {
    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[syncPointer], 60);
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int EecIv::answerRequestLiveData() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x41, 0x96 },
    {0x00, 0xa0 }
  };  

  if (pushAvailableToBuffer()) {
    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[syncPointer], 60);
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int EecIv::answerRequestFaultCodeShort() {
  unsigned char answerSig[2] = {0x01, 0xb0 };

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig, 60);
      
      syncPointer++;
      return 1;
    }
  }

  return 0;
}

int EecIv::readRequestFaultCode() { 

  if (pushAvailableToBuffer()) {
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


void EecIv::pushBuffer(unsigned char val) {
  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = val;
}

void EecIv::clearBuffer() {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = 0;
  }
}