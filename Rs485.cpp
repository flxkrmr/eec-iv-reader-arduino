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

int Rs485::read() {
  softwareSerial->read();
  //Serial.read();
}

int Rs485::available() {
  softwareSerial->available();
  //Serial.available();
}
