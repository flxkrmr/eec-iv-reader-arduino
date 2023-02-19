#include "Cart.h"

const uint8_t Cart::startMessage[12] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
    resetBuffer();
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
            delay.word = 1420;
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
        if (enableDiagnosticParameterSending && diagnosticParameterPointer == (frameNumber-1)*2) {
            //Serial.println("Sending diag params");
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

            // reset for next word
            wordBufferPointer = 0;
            resetBuffer();
            return;
        }

        switch(mode) {
            case WAIT_SYNC:
                // do nothing as we wait for next sync;
                break;
            case ID_SLOT:
                if (frameNumber > 15) {
                    frameNumber = 0;
                    isSynced = true;
                    frameDone = true;
                }

                memcpy(idSlot, wordBuffer, 16);

                // parity check
                if (((idSlot->rpm & 0xF) ^ ((idSlot->rpm >> 4) & 0xF) ^ idSlot->frameNumber ^ 0xA) != idSlot->parity) {
                    Serial.println("ID-Slot parity error");
                    frameNumber = 0;
                    mode = WAIT_SYNC;
                    break;
                }

                // if the we find the wrong frame number, we start again
                if (frameNumber != idSlot->frameNumber) {
                    frameNumber = 0;
                    mode = WAIT_SYNC;
                    break;
                }
                
                frameNumber++;

                if (idSlot->frameNumber < 4) {
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
                // only read one word and wait for next frame
                memcpy(data, wordBuffer, 2);
                hasData = true;
                mode = WAIT_SYNC;
                break;
        }

        // reset for next word
        wordBufferPointer = 0;
        resetBuffer();
    }

}

void Cart::handleStatusSlot() {
    switch (idSlot->frameNumber)  {
        case CURRENT_DIAGNOSTIC_MODE:
            currentDiagnosticMode = wordBuffer[0];
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

void Cart::resetBuffer() {
    wordBuffer[0] = 0xff;
    wordBuffer[1] = 0xff;
}