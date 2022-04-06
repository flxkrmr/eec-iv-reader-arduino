#include <Arduino.h>
#include <U8x8lib.h>
#include <EasyButton.h>

//using namespace Menu;

#include "EecIv.h"

#include <Wire.h>

// Pins RS485
#define DI 3
#define RE 6
#define RO 2

// Pins Buttons
#define BTN_1 7
#define BTN_2 8
#define BTN_3 9


EasyButton button1(BTN_1);
EasyButton button2(BTN_2);
EasyButton button3(BTN_3);

#define UP_SIGN '\x8c'
#define DOWN_SIGN '\x8e'
#define SELECT_SIGN '\xbb'
#define BACK_SIGN '\xab'
#define NO_SIGN ' '
#define NUM_COLUMN 16
#define NUM_ROW 8
#define HEADING_SELECT "Select Mode"

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void serialPrint(char message[]) {
  Serial.println(message);
}

void onFaultCodeFinished(char message[]);

EecIv eecIv = EecIv(DI, RO, RE);

#define NUM_MODES 3
enum MODE {
  FAULT_CODE,
  KOEO,
  LIVE_DATA
};
int mode = FAULT_CODE;

enum SCREEN_MODE {
  SELECT_MODE,
  READING_FAULT_CODE,
  RESULT_FAULT_CODE,
  RUNNING_KOEO,
  RESULT_KOEO
};
int screenMode = SELECT_MODE;


void setup() {
  Serial.begin(19200);
  Serial.println("### EEC IV Reader ###");

  Wire.begin();
  u8x8.begin();

  
  button1.begin();
  button1.onPressed(onButtonUp);
  button2.begin();
  button2.onPressed(onButtonSelect);
  button3.begin();
  button3.onPressed(onButtonDown);

  eecIv.debugPrint = &serialPrint;
  eecIv.onFaultCodeFinished = &onFaultCodeFinished;
  eecIv.onKoeoFinished = &onKoeoFinished;
  eecIv.setup();

  initSelectMode();
}

void drawWaitingScreen() {
  u8x8.clear();

  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(1, 3, "Reading...");
}

void drawMenuScreen(char selectSign, char upSign, char downSign, char heading[], char bodyLine1[], char bodyLine2[], char bodyLine3[]) {
  u8x8.clear();

  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.drawString(1, 0, heading); // 15
  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(0, 2, bodyLine1); // 15
  u8x8.drawString(0, 4, bodyLine2); // 15
  u8x8.drawString(0, 6, bodyLine3); // 14

  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.drawGlyph(NUM_COLUMN - 1, NUM_ROW/2 - 1, selectSign);
  u8x8.drawGlyph(NUM_COLUMN - 2, NUM_ROW - 1, upSign);
  u8x8.drawGlyph(NUM_COLUMN - 1, NUM_ROW - 1, downSign);
}

void loop() {
  button1.read();
  button2.read();
  button3.read();

  eecIv.mainLoop();
}

void onButtonUp() {
  switch(screenMode) {
    case SELECT_MODE:
      switchMode(false);
      break;
  }
}

void onButtonDown() {
  switch(screenMode) {
    case SELECT_MODE:
      switchMode(false);
      break;
  }
}

void onButtonSelect() {
  switch(screenMode) {
    case SELECT_MODE:
      selectMode();
      break;
    case RESULT_KOEO:
    case RESULT_FAULT_CODE:
      initSelectMode();
      break;
  }
}

void initSelectMode() {
  eecIv.setModeFaultCode();
  screenMode = SELECT_MODE;
  mode = FAULT_CODE;
  drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Read Fault-", "Code", "");
}

void switchMode(bool down) {
  mode = down ? (mode+NUM_MODES-1)%NUM_MODES : (mode+1)%NUM_MODES;
  switch (mode) {
    case FAULT_CODE:
      drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Read Fault-", "Code", "");
      break;
    case KOEO:
      drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Run KOEO", "", "");
      break;
    case LIVE_DATA:
      drawMenuScreen(NO_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Read Live-", "Data", "");
      break;
  }
}

void selectMode() {
  switch (mode) {
    case FAULT_CODE:
      eecIv.setModeFaultCode();
      eecIv.restartReading();
      screenMode = READING_FAULT_CODE;
      drawWaitingScreen();
      break;
    case KOEO:
      eecIv.setModeKoeo();
      eecIv.restartReading();
      screenMode = RUNNING_KOEO;
      drawWaitingScreen();
      break;
    case LIVE_DATA:
      break;
  }
}

void onKoeoFinished(char message[]) {
  screenMode = RESULT_KOEO;
  drawMenuScreen(BACK_SIGN, NO_SIGN, NO_SIGN, "Fault Code", message, "", "");
}

void onFaultCodeFinished(char message[]) {
  screenMode = RESULT_FAULT_CODE;
  drawMenuScreen(BACK_SIGN, NO_SIGN, NO_SIGN, "Fault Code", message, "", "");
}