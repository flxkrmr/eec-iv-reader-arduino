#ifndef EEC_IV_COMMON_H
#define EEC_IV_COMMON_H

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
};


#endif /* EEC_IV_COMMON_H */