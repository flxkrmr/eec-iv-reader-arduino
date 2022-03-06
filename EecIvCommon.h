#ifndef EEC_IV_COMMON_H
#define EEC_IV_COMMON_H

#include "arduino.h"
#include <SoftwareSerial.h>

class EecIvCommon {
  public:
  typedef void (*callback_t)(char []);

  protected:
  void answer(unsigned char message[], int delay);

  void rxMode(int baudrate);
  void enableWriteMode();
  void enableReadMode();
    
  int pin_re;
  SoftwareSerial *softwareSerial;

  const static unsigned char syncSig[4][4];
};


#endif /* EEC_IV_COMMON_H */