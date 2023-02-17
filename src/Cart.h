#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>

#define RE_READ 0x0
#define RE_WRITE 0x1

class Cart {

    public:
        bool hasData = false;
        void getData(uint8_t* data);

        void setDiagnosticParameter(const uint8_t diagnosticParameter[]);
        // when false, sending diagnostic parameter is disabled
        bool enableDiagnosticParameterSending = false;
        // when true, the diagnostic parameter has been send at least one time
        bool diagnosticParameterSendingDone = false;
        
        uint8_t currentDiagnosticMode = 0;

        void setBaudrate(long baudrate);
        void sendStartMessage();

        void loop();
        bool isSynced = false;

        // can be set to false, will be set true if current frame ends
        bool frameDone = true;

        Cart(SoftwareSerial* softwareSerial, uint8_t pin_re);
        void reset();

    private:

        enum Mode {
            WAIT_SYNC,
            ID_SLOT,
            DIAG_PARAM_SLOT,
            STATUS_SLOT,
            DATA_SLOT,
        } mode = WAIT_SYNC;

        struct delay_s {
            uint16_t word; // ns
            uint16_t byte; // ns
        } delay;

        const static uint8_t startMessage[18];

        SoftwareSerial *softwareSerial;
        uint8_t pin_re;

        void pushBuffer(uint8_t val);
        uint8_t pushAvailableToBuffer();
        void resetBuffer();

        uint8_t diagnosticParameter[8];
        uint8_t diagnosticParameterPointer = 0;

        uint8_t frameNumber = 0;
        bool isBufferSync();
        
        uint8_t wordBufferPointer = 0;
        uint8_t wordBuffer[2];

        uint8_t data[2];
};