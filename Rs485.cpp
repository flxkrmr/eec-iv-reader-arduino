#include "Rs485.h"

Rs485::Rs485(int de, int re) {
  pin_de = de;
  pin_re = re;

  softwareSerial = new SoftwareSerial(0, 1);
}

void Rs485::setup() {
  pinMode(pin_re,OUTPUT);
  pinMode(pin_de,OUTPUT);
}

void Rs485::sendStartMessage() {
  unsigned char start_message[18] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x00, 0x00,
    0x00, 0x05
  };
  
  softwareSerial->begin(2400);
  
  enableWriteMode();

  for(int i = 0; i<sizeof(start_message); i++) {
    softwareSerial->write(start_message[i]); // using softwareSerial as it can be timed better!
    delayMicroseconds(420); // try and error. Off delay has to be ~850 us
  }
}

void Rs485::rxMode(int baudrate) {
  Serial.end();
  softwareSerial->end();
  Serial.begin(baudrate, SERIAL_8N1);
  enableReadMode();
}

void Rs485::enableWriteMode() {
  digitalWrite(pin_re, HIGH);
  digitalWrite(pin_de, HIGH);
}

void Rs485::enableReadMode() {
  digitalWrite(pin_re, LOW);
  digitalWrite(pin_de, LOW);
}


int Rs485::mainLoop() {
  switch(currentState) {
    case IDLE:
      break;
    case SEND_START_MESSAGE:
      sendStartMessage();
      currentState = ENABLE_READING_FAST_SYNC;
      break;

    case ENABLE_READING_FAST_SYNC:
      rxMode(9600);
      currentState = WAIT_FAST_SYNC;
    case WAIT_FAST_SYNC:
      if (waitSyncLoop()) {
        currentState = ANSWER_FAST_SYNC;
      }
      break;

    case ANSWER_FAST_SYNC:
      if (answerFastSyncLoop()) {
        currentState = WAIT_FAST_SYNC_SHORT;
      }
      break;
    case WAIT_FAST_SYNC_SHORT:
      if (waitSyncLoopShort()) {
        currentState = ENABLE_READING_SLOW_SYNC;
      }
      break;
    case ENABLE_READING_SLOW_SYNC:
      rxMode(2400);
      currentState = ANSWER_SLOW_SYNC;
    case ANSWER_SLOW_SYNC:
      if(answerSlowSyncLoop()) {
        //currentState = IDLE;
      }
      break;
    case ANSWER_REQUEST:
    case READ_REQUEST:
      currentState = IDLE;
      break;
  }
}

int Rs485::waitSyncLoop() {
  unsigned char syncSig[4][4] = {
    {0x00, 0x00, 0x00, 0xa0 },
    {0x00, 0x00, 0x00, 0xb1 },
    {0x00, 0x00, 0x00, 0x82 },
    {0x00, 0x00, 0x00, 0x93 }
  };
    
  unsigned char read_buffer[4];

  if (Serial.available()) {
    if (Serial.readBytes(read_buffer, 4) != 4) {
      return 0;
    };

    if (!memcmp(syncSig[syncPointer], read_buffer, 4)) {      
      syncPointer++;

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
  unsigned char read_buffer[4];

  if (Serial.available()) {
    if (Serial.readBytes(read_buffer, 4) != 4) {
      return 0;
    };

    if (!memcmp(syncSig, read_buffer, 4)) {    
      return 1;
    }
  }

  return 0;
}

int Rs485::answerFastSyncLoop() {
  unsigned char syncSig[4][4] = {
    {0x00, 0x00, 0x00, 0xa0 },
    {0x00, 0x00, 0x00, 0xb1 },
    {0x00, 0x00, 0x00, 0x82 },
    {0x00, 0x00, 0x00, 0x93 }
  };

  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x81, 0x74 },
    {0x00, 0xa0 }
  };
  
  unsigned char read_buffer[4];
  
  if (Serial.available()) {
    if (Serial.readBytes(read_buffer, 4) != 4) {
      return 0;
    };

    if (!memcmp(syncSig[syncPointer], read_buffer, 4)) {
      delayMicroseconds(200);
      enableWriteMode();

      Serial.end();
      softwareSerial->begin(9600);
      softwareSerial->write(answerSig[syncPointer][0]);
      delayMicroseconds(15);
      softwareSerial->write(answerSig[syncPointer][1]);

      softwareSerial->end();

      Serial.begin(9600);

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

int Rs485::answerSlowSyncLoop() {
  unsigned char syncSig[4][4] = {
    {0x00, 0x00, 0x00, 0xa0 },
    {0x00, 0x00, 0x00, 0xb1 },
    {0x00, 0x00, 0x00, 0x82 },
    {0x00, 0x00, 0x00, 0x93 }
  };

  unsigned char answerSig[4][2] = {
    {0x01, 0xb0 },
    {0xff, 0x5f },
    {0x81, 0x74 },
    {0x00, 0xa0 }
  };
  
  unsigned char read_buffer[4];
  
  if (Serial.available()) {
    if (Serial.readBytes(read_buffer, 4) != 4) {
      return 0;
    };

    if (!memcmp(syncSig[syncPointer], read_buffer, 4)) {
      delayMicroseconds(800);
      enableWriteMode();

      Serial.end();
      softwareSerial->begin(2400);
      softwareSerial->write(answerSig[syncPointer][0]);
      delayMicroseconds(60);
      softwareSerial->write(answerSig[syncPointer][1]);

      softwareSerial->end();

      Serial.begin(2400);

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
