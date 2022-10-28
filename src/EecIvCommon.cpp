#include "EecIvCommon.h"

const unsigned char EecIvCommon::syncSig[4][4] = {
  {0x00, 0x00, 0x00, 0xa0 },
  {0x00, 0x00, 0x00, 0xb1 },
  {0x00, 0x00, 0x00, 0x82 },
  {0x00, 0x00, 0x00, 0x93 }
};

void EecIvCommon::answer(unsigned char message[], int delay) {
  enableWriteMode();
  softwareSerial->write(message[0]);
  delayMicroseconds(delay);
  softwareSerial->write(message[1]);
  enableReadMode(); 
}

void EecIvCommon::rxMode(int baudrate) {
  softwareSerial->begin(baudrate);
  enableReadMode();
}

void EecIvCommon::enableWriteMode() {
  digitalWrite(pin_re, HIGH);
}

void EecIvCommon::enableReadMode() {
  digitalWrite(pin_re, LOW);
}
