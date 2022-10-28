#include "Koeo.h"

Koeo::Koeo(SoftwareSerial *softwareSerial, int re, EecIvCommon::callback_t printCallback) {
  this->softwareSerial = softwareSerial;
  this->pin_re = re;
  this->print = printCallback;   
}

int Koeo::mainLoop() {
  switch(currentState) {
    case ANSWER_REQUEST_KOEO: // Answer sync with koeo command for 8 times
      if (answerRequestKoeo()) {
        loopCounter++;
        if (loopCounter > 7) {
          currentState = ANSWER_REQUEST_KOEO_SHORT;
          loopCounter = 0;
        }
      }
      break;
    case ANSWER_REQUEST_KOEO_SHORT: // Answer only one sync message with koeo command
      if (answerRequestKoeoShort()) {
        currentState = READ_REQUEST_KOEO;
      }
      break;
    case READ_REQUEST_KOEO: // Read koeo fault code
      if (readRequestKoeo()) {
        currentState = ANSWER_REQUEST_KOEO_SHORT;
        loopCounter++;
        if (loopCounter > 3) {
          loopCounter = 0;
          currentState = WAIT_REQUEST_KOEO_SHORT;
        }
      }
      break;
    case WAIT_REQUEST_KOEO_SHORT: // Wait for 6 bytes from ecu
      if (waitByte()) {
        loopCounter++;
        if (loopCounter > 5) {
          loopCounter = 0;
          currentState = READ_REQUEST_KOEO_AFTER_ANSWER;
        }
      }
      break;
    case READ_REQUEST_KOEO_AFTER_ANSWER: // Read koeo code and stop after 8 codes
      if (readRequestKoeo()) {
        currentState = WAIT_REQUEST_KOEO_SHORT;
        koeoCounter++;
        if (koeoCounter > 7) {
          koeoCounter = 0;
          currentState = ANSWER_REQUEST_KOEO;
          return 1;
        }
      }
      break;
  }

  return 0;
}

int Koeo::answerRequestKoeo() {
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

int Koeo::answerRequestKoeoShort() {
  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x25, 0x94 },
    {0x00, 0xa0 }
  };  

  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());

    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[syncPointer], 60);
      
      syncPointer++;
      if (syncPointer > 3) {
        syncPointer = 0;
      }
      return 1;
    }
  }

  return 0;
}

int Koeo::readRequestKoeo() {
  if (pushAvailableToBuffer()) {
    errorCodePointer++;

    if(errorCodePointer >= 2) {
      errorCodePointer = 0;

      sprintf(printBuffer, "Koeo Code: %01X%02X", buffer[3] & 0xF, buffer[2]);
      print(printBuffer);
      return 1;
    }
  }

  return 0;
}

int Koeo::pushAvailableToBuffer() {
  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());
    return 1;
  } else {
    return 0;
  }
}

int Koeo::waitByte() {
  if (softwareSerial->available()) {
    softwareSerial->read();
    return 1;
  }
  return 0;
}

void Koeo::pushBuffer(unsigned char val) {
  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = val;
}

void Koeo::initBuffer(unsigned char buffer[]) {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = 0;
  }
}