#include "FaultCode.h"

void doNothingHere(char []) {}

FaultCode::FaultCode(SoftwareSerial *softwareSerial, int re, callback_t printCallback) {
  this->softwareSerial = softwareSerial;
  this->pin_re = re;
  this->print = printCallback;

  this->onFaultCode = doNothingHere;
}

void FaultCode::setOnFaultCode(callback_t onFaultCode) {
  this->onFaultCode = onFaultCode;
}

int FaultCode::mainLoop() {
  switch(currentState) {
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
        currentState = ANSWER_REQUEST_FAULT_CODE;
        return 1;
      }
      break;
  }
  return 0;
}

int FaultCode::answerRequestFaultCode() {
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

int FaultCode::answerRequestFaultCodeShort() {
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

int FaultCode::readRequestFaultCode() { 

  if (pushAvailableToBuffer()) {
    errorCodePointer++;

    if(errorCodePointer >= 2) {
      errorCodePointer = 0;

      sprintf(printBuffer, "Error Code: %01X%02X", buffer[3] & 0xF, buffer[2]);
      print(printBuffer);
      onFaultCode(printBuffer);
      return 1;
    }
  }

  return 0;
}


// move to base class

void FaultCode::pushBuffer(unsigned char val) {
  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = val;
}

void FaultCode::initBuffer() {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = 0;
  }
}

int FaultCode::pushAvailableToBuffer() {
  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());
    return 1;
  } else {
    return 0;
  }
}
