#include "LiveData.h"

LiveData::LiveData(SoftwareSerial *softwareSerial, int re, EecIvCommon::callback_t printCallback) {
  this->softwareSerial = softwareSerial;
  this->pin_re = re;
  this->print = printCallback;
}

int LiveData::mainLoop() {
  switch(currentState) {
    case REQUEST_LIVE_DATA:
      if (requestLiveData()) {
        currentState = INIT_LIVE_DATA_MESSAGE_1;
      }
      break;
    case INIT_LIVE_DATA_MESSAGE_1:
      if (initLiveDataMessage1()) {
        currentState = INIT_LIVE_DATA_MESSAGE_2;
      }
      break;
    case INIT_LIVE_DATA_MESSAGE_2:
      if (initLiveDataMessage2()) {
        currentState = INIT_LIVE_DATA_MESSAGE_3;
      }
      break;
    case INIT_LIVE_DATA_MESSAGE_3:
      if (initLiveDataMessage3()) {
        currentState = INIT_LIVE_DATA_MESSAGE_4;
      }
      break;
    case INIT_LIVE_DATA_MESSAGE_4:
      if (initLiveDataMessage4()) {
        currentState = INIT_LIVE_DATA_MESSAGE_1;
        //return 1;
      }
      break;
      
  }

  return 0;
}


int LiveData::requestLiveData() {
  unsigned char requestLiveDataSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x41, 0x96 },
    {0x00, 0xa0 }
  };  

  if (pushAvailableToBuffer()) {
    if (!memcmp(syncSig[syncPointer], buffer, 4)) {
      delayMicroseconds(1420);
      answer(requestLiveDataSig[syncPointer], 60);
      
      syncPointer++;

      if (syncPointer > 3) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int LiveData::initLiveDataMessage1() {
  unsigned char readSig[4] = {
    0x00, 0x00, 0x00, 0xA0
  };

  unsigned char answerSig[6][2] = {
    {0x01, 0xb0 },
    {0x01 },
    {0x38, 0x11 },
    {0x28, 0x08 },
    {0xa8, 0x09 },
    {0xb8 }
  };  

  if (pushAvailableToBuffer()) {
    if (!memcmp(readSig, buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[0], 60);
      
      delayMicroseconds(5000);
      softwareSerial->write(answerSig[1][0]);

      delayMicroseconds(1420);
      answer(answerSig[2], 60);
      delayMicroseconds(1420);
      answer(answerSig[3], 60);
      delayMicroseconds(1420);
      answer(answerSig[4], 60);
      delayMicroseconds(1420);
      softwareSerial->write(answerSig[5][0]);

      return 1;
    }
  }

  return 0;
}

int LiveData::initLiveDataMessage2() {
  unsigned char readSig[4] = {
    0x00, 0x00, 0x00, 0xB1
  };

  unsigned char answerSig[6][2] = {
    {0xff, 0x5f },
    {0x0d },
    {0xf8, 0x10 },
    {0x38, 0x0f }, // ?
    {0xd8, 0x15 },
    {0x68 }
  };

  if (pushAvailableToBuffer()) {
    if (!memcmp(readSig, buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[0], 60);
      delayMicroseconds(5000);
      softwareSerial->write(answerSig[1][0]);

      delayMicroseconds(1420);
      answer(answerSig[2], 60);
      delayMicroseconds(1420);
      answer(answerSig[3], 60);
      delayMicroseconds(1420);
      answer(answerSig[4], 60);
      delayMicroseconds(1420);
      softwareSerial->write(answerSig[5][0]);
      return 1;
    }
  }

  return 0;
}

int LiveData::initLiveDataMessage3() {
  unsigned char readSig[4] = {
    0x00, 0x00, 0x00, 0x82
  };

  unsigned char answerSig[6][2] = {
    {0x21, 0xf6 },
    {0x26 },
    {0x68, 0x0c },
    {0xe8, 0x04 },
    {0x68, 0x0e }, // ?
    {0xc8 }
  }; 

  if (pushAvailableToBuffer()) {
    if (!memcmp(readSig, buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[0], 60);
      delayMicroseconds(5000);
      softwareSerial->write(answerSig[1][0]);

      delayMicroseconds(1420);
      answer(answerSig[2], 60);
      delayMicroseconds(1420);
      answer(answerSig[3], 60);
      delayMicroseconds(1420);
      answer(answerSig[4], 60);
      delayMicroseconds(1420);
      softwareSerial->write(answerSig[5][0]);
      return 1;
    }
  }

  return 0;
}

int LiveData::initLiveDataMessage4() {
  unsigned char readSig[4] = {
    0x00, 0x00, 0x00, 0x93
  };

  unsigned char answerSig[6][2] = {
    {0x00, 0xa0 },
    {0x16 },
    {0x58, 0x1b },
    {0x88, 0x1a },
    {0x98, 0x20 },
    {0x08 }
  }; 

  if (pushAvailableToBuffer()) {
    if (!memcmp(readSig, buffer, 4)) {
      delayMicroseconds(1420);
      answer(answerSig[0], 60);
      delayMicroseconds(5000);
      softwareSerial->write(answerSig[1][0]);

      delayMicroseconds(1420);
      answer(answerSig[2], 60);
      delayMicroseconds(1420);
      answer(answerSig[3], 60);
      delayMicroseconds(1420);
      answer(answerSig[4], 60);
      delayMicroseconds(1420);
      softwareSerial->write(answerSig[5][0]);
      return 1;
    }
  }

  return 0;
}

int LiveData::pushAvailableToBuffer() {
  if (softwareSerial->available()) {
    pushBuffer(softwareSerial->read());
    return 1;
  } else {
    return 0;
  }
}

void LiveData::pushBuffer(unsigned char val) {
  buffer[0] = buffer[1];
  buffer[1] = buffer[2];
  buffer[2] = buffer[3];
  buffer[3] = val;
}

void LiveData::initBuffer() {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = 0;
  }
}
