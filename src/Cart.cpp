#include "Cart.h"

const uint8_t Cart::startMessage[18] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x01, 0x04, 0x00, 0x00,
  0x00, 0x05
};

Cart::Cart(SoftwareSerial* softwareSerial, uint8_t pin_re) {
    this->softwareSerial = softwareSerial;
    this->pin_re = pin_re;
    pinMode(pin_re, OUTPUT);
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
    }

    isSynced = false;
}


void Cart::loop() {

    // if in diag param slot, we don't wait for bytes
    // just send the parameter and return
    if (mode == DIAG_PARAM_SLOT) {
        if (sentDiagnosticParameter && diagnosticParameterPointer == (frameNumber-1)*2) {
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
                diagnosticParameterSend = true;
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
                }

                // if the we find the wrong frame number, we start again
                if (frameNumber != (wordBuffer[1] & 0xF)) {
                    frameNumber = 0;
                    mode = WAIT_SYNC;
                    break;
                }
                // increase the frameNumber here.
                // like this we can be sure that we don't miss a frame.
                // downside is, that we always have to subtract one to check the frameNumber afterwards...
                frameNumber++;

                // for now don't care about the rest of the id slot

                if (frameNumber-1 < 4) {
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
                if (frameNumber-1 == 4)  {
                    currentDiagnosticMode = wordBuffer[0];
                }
                mode = DATA_SLOT;
                break;
            case DATA_SLOT:
                // only read one word and wait for next frame
                //onReadDataMessage(wordBuffer[0], wordBuffer[1]);
                mode = WAIT_SYNC;
                break;
        }

        // reset for next word
        wordBufferPointer = 0;
        resetBuffer();
    }

}

void Cart::setDiagnosticParameter(const uint8_t diagnosticParameter[]) {
    memcpy(this->diagnosticParameter, diagnosticParameter, 8);
    sentDiagnosticParameter = true;
    diagnosticParameterSend = false;
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