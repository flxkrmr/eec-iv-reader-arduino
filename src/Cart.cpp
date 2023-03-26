#include "Cart.h"

const uint8_t Cart::startMessage[11] = {
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x04, 0x00, 0x00, 0x00, 0x05
};

Cart::Cart(SoftwareSerial* softwareSerial, uint8_t pin_re) {
    this->softwareSerial = softwareSerial;
    this->pin_re = pin_re;
    pinMode(pin_re, OUTPUT);

    reset();
}

void Cart::reset() {
    currentDiagnosticMode = 0;
    isSynced = false;
    frameDone = true;
    enableDiagnosticParameterSending = false;
    diagnosticParameterSendingDone = false;
    hasData = false;

    mode = WAIT_SYNC;
    diagnosticParameterPointer = 0;
    frameNumber = 0;
    wordBufferPointer = 0;
    
    digitalWrite(pin_re, RE_READ);
}

void Cart::sendStartMessage() { 
    digitalWrite(pin_re, RE_WRITE);

    for(uint8_t i = 0; i<sizeof(startMessage); i++) {
        softwareSerial->write(startMessage[i]);
        delayMicroseconds(420);
    }
    
    digitalWrite(pin_re, RE_READ);
}

void Cart::setBaudrate(long baudrate) {
    softwareSerial->begin(baudrate);
    switch (baudrate) {
        case 2400:
            delay.word = 1700;
            delay.byte = 60;
            break;
        case 9600:
            delay.word = 426;
            delay.byte = 15;
            break;
        case 19200:
            delay.word = 213;
            delay.byte = 7;
            break;
    }

    isSynced = false;
}


void Cart::loop() {
    // if in diag param slot, we don't wait for bytes
    // just send the parameter and return
    if (mode == DIAG_PARAM_SLOT) {
        if (enableDiagnosticParameterSending && diagnosticParameterPointer == idSlot.frameNumber*2) {
            delayMicroseconds(delay.word);
            digitalWrite(pin_re, RE_WRITE);
            softwareSerial->write(diagnosticParameter[diagnosticParameterPointer]);
            delayMicroseconds(delay.byte);
            softwareSerial->write(diagnosticParameter[diagnosticParameterPointer+1]);

            digitalWrite(pin_re, RE_READ);

            diagnosticParameterPointer+=2;

            if (diagnosticParameterPointer > 7) {
                diagnosticParameterPointer = 0;
                diagnosticParameterSendingDone = true;
            }

        }

        mode = DATA_SLOT;
        return;
    }

    if (mode == DATA_SLOT) {
        if (enablePidMapSending && pidMapPointer == idSlot.frameNumber && pidMapPointer < 12) {
            digitalWrite(pin_re, RE_WRITE);
            for (uint8_t i = 0; i < 4; i++) {
                uint8_t pid = pidMap[pidMapPointer][i];
                delayMicroseconds(delay.word);
                softwareSerial->write(pid);
                delayMicroseconds(delay.byte);
                softwareSerial->write(PID_CHECKSUM(pid));
            }
            pidMapPointer++;

            if (pidMapPointer >= 12) {
                pidMapSendingDone = true;
                enablePidMapSending = false;
                pidMapPointer = 0;
            }
            digitalWrite(pin_re, RE_READ);

            mode = WAIT_SYNC;
        }
    }

    // read next byte and check if we have a full word already
    if (pushAvailableToBuffer()) {

        wordBufferPointer++;
        if (wordBufferPointer < 2) {
            // buffer not full, wait for next byte
            return;
        }

        // always look out for sync word and stop everything else if
        // we find one
        if (isBufferSync()) {
            // look for ID slot
            mode = ID_SLOT;

            //Serial.print('.');

            // reset for next word
            wordBufferPointer = 0;
            return;
        }

        /*
        char debugBuf[24];
        sprintf(debugBuf, "0x%02X 0x%02X", wordBuffer[0], wordBuffer[1]);
        Serial.println(debugBuf);
        */

        switch(mode) {
            case WAIT_SYNC:
                // do nothing as we wait for next sync.
                // return not break because the current word might contain part of a
                // sync word.
                return;
            case ID_SLOT:

                if (frameNumber > 15) {
                    frameNumber = 0;
                    isSynced = true;
                    frameDone = true;
                }

                memcpy(&idSlot, wordBuffer, 2);

                // parity check
                if (((idSlot.rpm & 0xF) ^ ((idSlot.rpm >> 4) & 0xF) ^ idSlot.frameNumber ^ 0xA) != idSlot.parity) {
                    Serial.println("ID-Slot parity error");
                    frameNumber = 0;
                    mode = WAIT_SYNC;
                    // only return and not reset buffer and buffer pointer. If there was a 0x00 in the buffer
                    // we would miss this for syncing
                    return;
                }

                // if the we find the wrong frame number, we start again
                if (frameNumber != idSlot.frameNumber) {
                    //Serial.println("Frame number error");
                    frameNumber = 0;
                    mode = WAIT_SYNC;
                    return;
                }
                
                frameNumber++;
                dataWordCounter = 0;

                if (idSlot.frameNumber < 4) {
                    mode = DIAG_PARAM_SLOT;
                } else {
                    mode = STATUS_SLOT;
                }
                break;
            case DIAG_PARAM_SLOT:
                // this cannot be reached, as we don't care about word
                // reading in this mode
                break;
            case STATUS_SLOT:
                handleStatusSlot();
                mode = DATA_SLOT;
                break;
            case DATA_SLOT:
                dataWordCounter++;
                // prevent beeing stuck in capturing live data
                // there is only space for 5 words
                if (dataWordCounter > 5) {
                    mode = WAIT_SYNC;
                    break;
                }
                memcpy(data, wordBuffer, 2);
                hasData = true;
                break;
        }

        // reset for next word
        wordBufferPointer = 0;
    }

}

void Cart::handleStatusSlot() {
    // TODO check parity 
    switch (idSlot.frameNumber)  {
        case CURRENT_DIAGNOSTIC_MODE:
            currentDiagnosticMode = wordBuffer[0];
            break;
        case NEXT_DIAGNOSTIC_MODE:
            nextDiagnosticMode = wordBuffer[0];
            break;
        case DCL_ERROR_FLAG_LOW:
            memcpy(&dclErrorFlagLow, wordBuffer, 1);
            break;
        case DCL_ERROR_FLAG_HIGH:
            memcpy(&dclErrorFlagHigh, wordBuffer, 1);
            break;
    }
}

void Cart::getData(uint8_t *data) {
    memcpy(data, this->data, 2);
    hasData = false;
}

void Cart::setDiagnosticParameter(const uint8_t diagnosticParameter[]) {
    memcpy(this->diagnosticParameter, diagnosticParameter, 8);
    enableDiagnosticParameterSending = true;
    diagnosticParameterSendingDone = false;
}

void Cart::setPidMap(const uint8_t pidMap[12][4]) {
    memcpy(this->pidMap, pidMap, sizeof(this->pidMap));
    enablePidMapSending = true;
    pidMapSendingDone = false;
}

bool Cart::isBufferSync() {
    return !(wordBuffer[0] | wordBuffer[1]);
}

uint8_t Cart::pushAvailableToBuffer() {
    if (softwareSerial->available()) {
        pushBuffer(softwareSerial->read());
        return 1;
    } else {
        return 0;
    }
}

void Cart::pushBuffer(uint8_t val) {
    wordBuffer[0] = wordBuffer[1];
    wordBuffer[1] = val;
}