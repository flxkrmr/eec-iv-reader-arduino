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

void Rs485::txStartMessage() {
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

void Rs485::rxMode9600() {
  softwareSerial->end();
  Serial.begin(9600, SERIAL_8N1);
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

int Rs485::syncLoop(int answer) {
  int syncPointerMax = 3;
  
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
  
  if (Serial.available()) {
    if (Serial.readBytes(read_buffer, 4) != 4) {
      return 0;
    };

    if (!memcmp(syncSig[syncPointer], read_buffer, 4)) {
      if (answer && syncPointer < 4) {
        enableWriteMode();
        //Serial.write(answerSig[syncPointer], sizeof(answerSig[syncPointer]));
        Serial.end();
        softwareSerial->begin(9600);
        softwareSerial->write(answerSig[syncPointer][0]);
        //delayMicroseconds(105);
        //delayMicroseconds(100);
        //delayMicroseconds(95);
        //delayMicroseconds(90);
        //delayMicroseconds(85);
        //delayMicroseconds(80);
        delayMicroseconds(406);
        softwareSerial->write(answerSig[syncPointer][1]);
        //softwareSerial->write(answerSig[syncPointer], 2);

        softwareSerial->end();

        Serial.begin(9600);

        enableReadMode(); 
      }
      
      //emptyFilo();
      syncPointer++;

      if (syncPointer > syncPointerMax) {
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

int Rs485::read() {
  return Serial.read();
}

int Rs485::available() {
  return Serial.available();
}
