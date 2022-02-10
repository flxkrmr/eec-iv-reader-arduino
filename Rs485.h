#ifndef RS_485_H
#define RS_485_H

#include "arduino.h"
#include <SoftwareSerial.h>

class Rs485 {
  public:
  Rs485(int de, int re);
  void setup();
  
  void txStartMessage();
  void rxMode9600();

  int syncLoop(int answer);

  int read();
  int available();

  private:
  int pin_de;
  int pin_re;

  char out_buffer[64];

  unsigned char read_buffer[4];
  int syncPointer = 0;

  void enableWriteMode();
  void enableReadMode();

  SoftwareSerial *softwareSerial;
};

#endif /* RS_485 */
