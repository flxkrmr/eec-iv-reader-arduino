#include "EecIv.h"

EecIv::EecIv(int di, int ro, int re) {
  cart = new Cart(new SoftwareSerial(di, ro), re);
}

const uint8_t EecIv::pidMap[12][4] = {
  {
    0x01, // Engine Revolution
    0x02, // Manifold Absolute Pressure
    0x03, // Barometric Pressure
    0x04, // Total spark advance
  }, {
    0x05, // ACT Sensor
    0x06, // ECT Sensor
    0x07, // EVP or EPT sensor
    0x08, // Lambda (IEGO1)
  }, {
    0x09, // Throttle Position
    0x0A, // Not Used?
    0x0B, // Calibration input voltage
    0x0C, // Fuel Pulsewidth
  }, {
    0x0D, // LAMBDA equivalence ratio
    0x0E, // throttle mode
    0x0F, // air temp (c)
    0x10, // ECT Fahrenheit
  }, {
    0x11, // Supply Voltage
    0x12, // SCAP sensor
    0x13, // EGR Duty Cycle
    0x14, // Not Used
  }, {
    0x15, // idle speed duty
    0x16, // airflow meter
    0x17, // speed
    0x18, // Speed??
  }, {
    0x19,
    0x1A, // bitmap 0
    0x1B, // bitmap 1
    0x1C, // Not Used
  }, {
    0x1D, // Not used
    0x26, // Not used
    0x27, // Normalized air charge value
    0x28, // Adaptive fuel correction.
  }, {
    0x29, // Not used
    0x2A, // Desired rpm.
    0x2B, // Ratch
    0x2C, // Not used
  }, {
    0x2D, // Time since startup
    0x2E, // OCC A/D input level.
    0x2F, // Neutral / drive input.
    0x30, // Converter clutch commanded state
  }, {
    0x31, // Not used
    0x32, // Commanded gear for shift solenoids.
    0x33, // Not used
    0x34, // Not used
  }, {
    0x35, // Actual ETV monitor voltage, counts
    0x36, // EPC pressure, psi.
    0x37, // Transmission Oil temperature, counts.
    0x38 // Current PRNDL position.
  }
};

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

        uint8_t data[2];
        cart->getData(data);
        onFaultCodeRead(data);
      }

      if (koeoCounter > 0 && cart->frameDone) {
        koeoCounter = 0;
        onFaultCodeFinished();
        currentState = IDLE;
        cart->enableDiagnosticParameterSending = false;
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

      cart->setDiagnosticParameter(pidMessage);
      cart->setPidMap(pidMap);
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
          liveDataOffset = 0;
        }
        cart->getData(liveDataBuf+liveDataOffset);

        liveDataOffset += 2;

        if (liveDataOffset >= 8) {
          uint8_t liveDataWithName[12];
          for (uint8_t i = 0; i < 4; i++) {
            liveDataWithName[i*3 + 0] = pidMap[liveDataLastFrame][i];
            liveDataWithName[i*3 + 1] = liveDataBuf[i*2 + 0];
            liveDataWithName[i*3 + 2] = liveDataBuf[i*2 + 1];
          }
          Serial.write("\x01\x23\x03", 3);
          Serial.write(liveDataWithName, 12);
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

