#include "Rs485.h"

Rs485::Rs485(int de, int re) {
  pin_de = de;
  pin_re = re;

  softwareSerial = new SoftwareSerial(2, 3);
}

void Rs485::setup() {
  pinMode(pin_re,OUTPUT);
  pinMode(pin_de,OUTPUT);
}

void Rs485::txStartMessage() {
  unsigned char start_message[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x01, 0x04, 0x00, 0x00,
    0x00, 0x05
  };
  
  //Serial.begin(2400, SERIAL_8N2);
  softwareSerial->begin(2400);
  
  enableWriteMode();
  //Serial.write(start_message, sizeof(start_message));
  //Serial.write("hello");
  //Serial.flush();
  softwareSerial->write(start_message, sizeof(start_message));
}
void Rs485::rxMode9600() {
  //Serial.begin(9600, SERIAL_8N2);
  softwareSerial->begin(9600);
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

int Rs485::test() {
  Serial.begin(19200);
  Serial.write("waiting for sync\n");

  return 1;
}

int Rs485::syncLoop() {
  Serial.begin(19200);
  //Serial.write("waiting for sync\n");
  //int syncPointer = 0;
  int syncPointerMax = 15;
  
  unsigned char syncSig[16][4] = {
    {0x00, 0x00, 0x00, 0xa0 },
    {0x00, 0x00, 0x00, 0xb1 },
    {0x00, 0x00, 0x00, 0x82 },
    {0x00, 0x00, 0x00, 0x93 },
    {0x00, 0x00, 0x00, 0xe4 },
    {0x00, 0x00, 0x00, 0xf5 },
    {0x00, 0x00, 0x00, 0xc6 },
    {0x00, 0x00, 0x00, 0xd7 },
    {0x00, 0x00, 0x00, 0x28 },
    {0x00, 0x00, 0x00, 0x39 },
    {0x00, 0x00, 0x00, 0x0a },
    {0x00, 0x00, 0x00, 0x1b },
    {0x00, 0x00, 0x00, 0x6c },
    {0x00, 0x00, 0x00, 0x7d },
    {0x00, 0x00, 0x00, 0x4e },
    {0x00, 0x00, 0x00, 0x5f }
  };

  if (softwareSerial->available()) {
    putFilo(softwareSerial->read());
    sprintf(out_buffer, "Is: %02X %02X %02X %02X\n", filo[0], filo[1], filo[2], filo[3]);
    Serial.write(out_buffer);
    if (!memcmp(syncSig[syncPointer], filo, 4)) {
      sprintf(out_buffer, "Found: %02X %02X %02X %02X\n", syncSig[syncPointer][0], syncSig[syncPointer][1], syncSig[syncPointer][2], syncSig[syncPointer][3]);
      Serial.write(out_buffer);
      syncPointer++;
      emptyFilo();

      if (syncPointer > syncPointerMax) {
        Serial.write("found something");
        syncPointer = 0;
        return 1;
      }
    }
  }

  return 0;
}

void Rs485::emptyFilo() {
  filo[0] = 0x00;
  filo[1] = 0x00;
  filo[2] = 0x00;
  filo[3] = 0x00; 
}

void Rs485::putFilo(int value) {
  filo[0] = filo[1];
  filo[1] = filo[2];
  filo[2] = filo[3];
  filo[3] = value;   
}

int Rs485::read() {
  return softwareSerial->read();
  //Serial.read();
}

int Rs485::available() {
  return softwareSerial->available();
  //Serial.available();
}
