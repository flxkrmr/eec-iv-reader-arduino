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
          currentState = IDLE;
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
        Serial.println("Has Data");
        uint8_t data[2];
        cart->getData(data);
        onKoeoReadCode(data);
      }
      if (cart->dclErrorFlagHigh.selfTestComplete) {
        Serial.println("Self Test complete");
  
        onKoeoFinished();
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

        if (cart->frameDone || koeoCounter >= 12) {
          onKoeoFinished();
          currentState = IDLE;
          // TODO test 
          // cart->enableDiagnosticParameterSending = false;
        }
        
        uint8_t data[2];
        cart->getData(data);
        onKoeoReadCode(data);
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


int EecIv::pushAvailableToBuffer() {
  return 0;
}

bool EecIv::isBufferSync(uint8_t syncPointer) {
  return buffer[0] == 0x00 && 
    buffer[1] == 0x00 && // first two are always 0x00
    //buffer[2] == 0x00 && // this is 0x25 or 0x26 when engine is running, othewise 0x00...
    //(buffer[3] & 0xF0) != 0x00 && // chek for != 0 for snycPointer == 0
    (buffer[3] & 0x0F) == syncPointer;
}

void EecIv::pushBuffer(uint8_t val) {
  for (uint8_t i = 0; i < sizeof(buffer)-1; i++) {
    buffer[i] = buffer[i+1];
  }
  buffer[sizeof(buffer)-1] = val;
}

void EecIv::resetBuffer() {
  for (uint8_t i = 0; i < sizeof(buffer)-1; i++) {
    buffer[i] = 0xFF;
  }
}
