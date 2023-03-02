#include "EecIv.h"

EecIv::EecIv(int di, int ro, int re) {
  cart = new Cart(new SoftwareSerial(di, ro), re);
}

void EecIv::restartReading() {
  debugPrint("Check if ECU is in diag mode");
  currentState = CHECK_IF_IN_DIAG_MODE;
}

void EecIv::setMode(EecIv::OperationMode mode) {
  this->mode = mode;
}

void EecIv::mainLoop() {
  switch(currentState) {
    case IDLE:
      startMessageCounter = 0;
      break;

    case CHECK_IF_IN_DIAG_MODE:
      cart->reset();
      cart->setBaudrate(2400);
      initTimeoutTimer();
      currentState = WAIT_FOR_SYNC_2400;
      break;

    case SEND_START_MESSAGE:
      if(startMessageCounter >= startMessageCounterMax) {
        onStartMessageTimeout();
        currentState = IDLE;
        break;
      }
      cart->setBaudrate(2400);
      cart->sendStartMessage();

      startMessageCounter++;
      debugPrint("Send start message");
      currentState = CHANGE_BAUD_RATE_9600;
      break;

    case CHANGE_BAUD_RATE_9600:
      cart->setBaudrate(9600);
      //cart->setBaudrate(19200);
      initTimeoutTimer();
      currentState = WAIT_FOR_SYNC_9600;
      break;
    case WAIT_FOR_SYNC_9600:
      if (cart->isSynced) {
        debugPrint("Synced with default baud");
        startMessageCounter = 0;
        currentState = REQUEST_BAUD_RATE_CHANGE;
      } else {
        if(exceededTimeout()) {
          debugPrint("Exceeded waiting for sync in default baud");
          currentState = SEND_START_MESSAGE;
        }
      }
      break;

    case REQUEST_BAUD_RATE_CHANGE:
    {
      const uint8_t baudMessage[8] = {
        0x01, 0xb0, 0xff, 0x5f, 0x81, 0x74, 0x00, 0xa0 
      };
      cart->setDiagnosticParameter(baudMessage);
      currentState = WAIT_REQUEST_BAUD_RATE_CHANGE_DONE;
      break;
    }
    case WAIT_REQUEST_BAUD_RATE_CHANGE_DONE:
      if (cart->diagnosticParameterSendingDone) {
        currentState = CHANGE_BAUD_RATE_2400;
      }
      break;
    case CHANGE_BAUD_RATE_2400:
      cart->setBaudrate(2400);
      initTimeoutTimer();
      currentState = WAIT_FOR_SYNC_2400;
      break;
    case WAIT_FOR_SYNC_2400:
      if (cart->isSynced) {
        debugPrint("Synced with 2400");
        currentState = REQUEST_CLEAR_DCL_ERRORS;
      } else {
          if(exceededTimeout()) {
          debugPrint("Exceeded waiting for sync 2400");
          currentState = SEND_START_MESSAGE;
        }
      }
      break;
    case REQUEST_CLEAR_DCL_ERRORS:
    {
      const uint8_t clearDclMessage[8] = {
        0x01, 0xb0, 0xff, 0x5f, 0x01, 0xf4, 0x00, 0xa0 
      };
      cart->setDiagnosticParameter(clearDclMessage);
      currentState = WAIT_REQUEST_CLEAR_DCL_ERRORS;
      break;
    }
    case WAIT_REQUEST_CLEAR_DCL_ERRORS:
      if (cart->currentDiagnosticMode == 0x01) {
        debugPrint("DCL Errors cleared");
        if (mode == READ_FAULTS) {
          currentState = REQUEST_CONT_SELF_TEST_CODES;
        } else if (mode == KOEO) {
          currentState = REQUEST_KOEO;
        } else if (mode == LIVE_DATA) {
          currentState = REQUEST_PID_MODE;
        } else {
          currentState = IDLE;
        }
      }
      break;

    case REQUEST_CONT_SELF_TEST_CODES:
    {
      const uint8_t contSelfTestCodesMessage[8] = {
        0x01, 0xb0, 0xff, 0x5f, 0x26, 0xa4, 0x00, 0xa0 
      };
      cart->setDiagnosticParameter(contSelfTestCodesMessage);
      currentState = WAIT_REQUEST_CONT_SELF_TEST_CODES;
      break;
    }
    case WAIT_REQUEST_CONT_SELF_TEST_CODES:
      if (cart->nextDiagnosticMode == 0x26) {
        currentState = READ_CONT_SELF_TEST_CODES;
        cart->hasData = false;
      }
      break;
    case READ_CONT_SELF_TEST_CODES:
      if (cart->hasData) {
        uint8_t data[2];
        cart->getData(data);
        onFaultCodeRead(data);
      }
      if (cart->dclErrorFlagHigh.selfTestComplete) {
        Serial.println("Self Test complete");
  
        onFaultCodeFinished();
        cart->enableDiagnosticParameterSending = false;
        currentState = IDLE;
      }
      break;

    case REQUEST_KOEO:
    {
      const uint8_t koeoMessage[8] = {
        0x01, 0xb0, 0xff, 0x5f, 0x25, 0x94, 0x00, 0xa0 
      };
      cart->setDiagnosticParameter(koeoMessage);
      currentState = WAIT_REQUEST_KOEO;
      break;
    }
    case WAIT_REQUEST_KOEO:
      if (cart->currentDiagnosticMode == 0x25) {
        currentState = READ_KOEO;
        cart->hasData = false;
      }
      break;
    case READ_KOEO:
      if (cart->hasData) {
        koeoCounter++;

        // set flag to know when current frame ends
        if (koeoCounter == 1) {
          cart->frameDone = false;
        }

        if (cart->frameDone) {
          onFaultCodeFinished();
          currentState = IDLE;
          cart->enableDiagnosticParameterSending = false;
        }
        
        uint8_t data[2];
        cart->getData(data);
        onFaultCodeRead(data);
      }
      break;
    case REQUEST_PID_MODE:
    {
      const uint8_t pidMessage[8] = {
        0x01, 0xb0, 0xff, 0x5f, 0x41, 0x96, 0x00, 0xa0 
      };
      cart->setDiagnosticParameter(pidMessage);
      currentState = WAIT_PID_MODE;
      break;
    }
    case WAIT_PID_MODE:
      if (cart->nextDiagnosticMode == 0x41) {
        currentState = TRANSMIT_PID_MAP;
      }
      break;
    case TRANSMIT_PID_MAP:
    {
      const uint8_t pidMessage[8] = {
        0x01, 0xb0, 0xff, 0x5f, 0x21, 0xF6, 0x00, 0xa0 
      };

      const uint8_t pidMap[32] = {
        0x01, 0x38, // RPM geht
        0x08, 0xA8, // lambda geht
        0x11, 0x28, //  V supply geht
        0x09, 0xB8,  // throttle geht
        0x0D, 0xF8,  // short fuel
        0x0E, 0xC8,  // throttle mode
        0x06, 0x48,  // coolant sensor
        0x10, 0x38,   // coolant (c)
        0x05, 0x78,  // air temp sensor
        0x0F, 0xD8,  // air temp (c)
        0x15, 0x68,  // idle valve
        0x26, 0x68,   // airflow meter
        0x07, 0x58,  // EGR diff press
        0x0C, 0xE8,  // injection pulse
        0x04, 0x68,  // ignition timing
        0x0B, 0x98  // sensor power voltage
        //0x17, 0x48,   // speed
        //0x1A, 0x98,  // fuel vapor mode
        //0x1B, 0x88,  // fuel pump mode
        //0x16, 0x58  // lambda circuit mode
      };
      cart->setDiagnosticParameter(pidMessage);
      cart->setPidMap(pidMap, sizeof(pidMap));
      currentState = WAIT_TRANSMIT_PID_MAP;
      break;
    }
    case WAIT_TRANSMIT_PID_MAP:
      if (cart->pidMapSendingDone) {
        Serial.println("PID done");
        currentState = WAIT_PID_DATA;
      }
      break;
    case WAIT_PID_DATA:
      if (cart->hasData) {
        // frame changed so this will be the first data bit
        if (liveDataLastFrame != cart->idSlot.frameNumber) {
          liveDataLastFrame = cart->idSlot.frameNumber;
          if (cart->idSlot.frameNumber == 0) {
            liveDataOffset = 0;
          }
        }
        cart->getData(liveDataBuf+liveDataOffset);


        liveDataOffset += 2;

        if (liveDataOffset >= 32 || cart->idSlot.frameNumber > 4) {

          char outBuf[20];
          sprintf(outBuf, "Live Data");
          Serial.println(outBuf);
          Serial.write(liveDataBuf, 32);
          Serial.println();
          liveDataOffset = 0;
        }
      }
      break;
  }

  cart->loop();
}

int EecIv::exceededTimeout() {
  if (millis() > timeoutTimer + timeoutMax) {
    return 1;
  }
  return 0;
}

void EecIv::initTimeoutTimer() {
  timeoutTimer = millis();
}

