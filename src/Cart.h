#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>

#define RE_READ 0x0
#define RE_WRITE 0x1


#define PID_CHECKSUM(pid) ((((pid & 0xF) ^ ((pid >> 4) & 0xF) ^ 0x8 ^ 0xA) << 4 ) | 0x8)

class Cart {

    public:

        struct IdSlot {
            unsigned int rpm : 8;
            unsigned int frameNumber : 4;
            unsigned int parity : 4;
        } idSlot;

        struct DclErrorFlagLow {
            unsigned int loadAddrPartiy : 1;
            unsigned int loadAddrBadValue : 1;
            unsigned int dataChecksumPartiy : 1;
            unsigned int incorrectChecksum : 1;
            unsigned int adValuesParityError : 1;
            unsigned int pidMapParityError : 1;
            unsigned int dmrMapParityError : 1;
            unsigned int unused : 1;
        } dclErrorFlagLow;

        struct DclErrorFlagHigh {
            unsigned int unused : 2;
            unsigned int executeVectorParityError : 1;
            unsigned int executeVectorIncorrectChecksum : 1;
            unsigned int badDiagParameterSlot : 1;
            unsigned int eecInReset : 1;
            unsigned int selfTestComplete : 1;
            unsigned int background : 1;
        } dclErrorFlagHigh;

        bool hasData = false;
        void getData(uint8_t* data);
        uint8_t dataWordCounter = 0;

        void setDiagnosticParameter(const uint8_t diagnosticParameter[]);
        // when false, sending diagnostic parameter is disabled
        bool enableDiagnosticParameterSending = false;
        // when true, the diagnostic parameter has been send at least one time
        bool diagnosticParameterSendingDone = false;

        void setPidMap(const uint8_t pidMap[], size_t size);
        bool enablePidMapSending = false;
        bool pidMapSendingDone = false;
        
        uint8_t currentDiagnosticMode = 0;
        uint8_t nextDiagnosticMode = 0;

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

        enum StatusSlotType {
            CURRENT_DIAGNOSTIC_MODE = 0x4,
            NEXT_DIAGNOSTIC_MODE = 0x5,
            DCL_ERROR_FLAG_LOW = 0x6,
            DCL_ERROR_FLAG_HIGH = 0x7,
            DMR_LOW = 0x8,
            DMR_HIGH = 0x9,
            ROM_ID_LOG = 0xA,
            ROM_ID_HIGH = 0xB,
        };

        const static uint8_t startMessage[11];

        SoftwareSerial *softwareSerial;
        uint8_t pin_re;

        void pushBuffer(uint8_t val);
        uint8_t pushAvailableToBuffer();

        void handleStatusSlot();

        uint8_t diagnosticParameter[8];
        uint8_t diagnosticParameterPointer = 0;

        uint8_t pidMap[48];
        uint8_t pidMapPointer = 0;

        uint8_t frameNumber = 0;
        bool isBufferSync();
        
        uint8_t wordBufferPointer = 0;
        uint8_t wordBuffer[2];

        uint8_t data[2];
};