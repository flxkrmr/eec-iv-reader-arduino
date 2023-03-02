#include <Arduino.h>
#include <U8x8lib.h>
#include <OneButton.h>

#include "EecIv.h"

extern "C" {
  #include "version.h"
}

// Pins RS485
#define DI 2
#define RE 3
#define RO 4
// first board
//#define DI 3
//#define RE 6
//#define RO 2

// Pins Buttons
#define BTN_1 7
#define BTN_2 8
#define BTN_3 9


OneButton button1(BTN_1, false, false);
OneButton button2(BTN_2, false, false);
OneButton button3(BTN_3, false, false);

#define UP_SIGN '\x8c'
#define DOWN_SIGN '\x8e'
#define SELECT_SIGN '\xbb'
#define BACK_SIGN '\xab'
#define NO_SIGN ' '
#define NUM_COLUMN 16
#define NUM_ROW 8
#define HEADING_SELECT "Select Mode"


U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void serialPrint(const char message[]) {
  Serial.println(message);
}

void onFaultCodeRead(const uint8_t message[]);
void onFaultCodeFinished();
void onStartMessageTimeout();


char liveDataBuf[32];
void onLiveData(const uint8_t data[]);

void onButtonUp();
void onButtonDown();
void onButtonSelect();

void selectMode();
void showMainMenu();
void switchFaultCode(bool down);
void switchMainMenuMode(bool down);

void drawWelcomeScreen();
void drawMenuScreen(const char selectSign, const char upSign, const char downSign, const char heading[], const char bodyLine1[], const char bodyLine2[], const char bodyLine3[]);


EecIv eecIv (DI, RO, RE);

#define NUM_MAIN_MENU_MODES 3
enum MAIN_MENU_MODE {
  FAULT_CODE,
  KOEO,
  LIVE_DATA
};

MAIN_MENU_MODE mainMenuMode = FAULT_CODE;

enum SCREEN_MODE {
  MAIN_MENU,
  START_MESSAGE_TIMEOUT,
  READING_FAULT_CODE,
  RESULT_FAULT_CODE,
  RUNNING_KOEO,
  RESULT_KOEO
};
SCREEN_MODE screenMode = MAIN_MENU;


uint8_t fault_codes[12][2];
uint8_t fault_code_pointer = 0;
bool fault_codes_done = false;
uint8_t fault_code_display_pointer = 0; // index for showing koeo
char koeo_i_max = -1; // maximum index of koeos, -1 if none found

void setup() {
  Serial.begin(19200);

  u8x8.begin();

  button1.attachClick(onButtonUp);
  button2.attachClick(onButtonSelect);
  button3.attachClick(onButtonDown);

  eecIv.debugPrint = &serialPrint;
  eecIv.onFaultCodeRead = &onFaultCodeRead;
  eecIv.onFaultCodeFinished = &onFaultCodeFinished;
  eecIv.onStartMessageTimeout = &onStartMessageTimeout;

  eecIv.onLiveData = &onLiveData;

  drawWelcomeScreen();
  delay(2000);
  showMainMenu();
}

void drawWelcomeScreen() {
  u8x8.clear();
  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(1, 3, "EEC IV Reader");
  u8x8.drawString(1, 5, VERSION);
}

void drawWaitingScreen() {
  u8x8.clear();

  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(1, 3, "Reading...");
}

void drawMenuScreen(const char selectSign, const char upSign, const char downSign, const char heading[], const char bodyLine1[], const char bodyLine2[], const char bodyLine3[]) {
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
  button1.tick();
  button2.tick();
  button3.tick();

  eecIv.mainLoop();
}

void onButtonUp() {
  switch(screenMode) {
    case MAIN_MENU:
      switchMainMenuMode(false);
      break;
    case RESULT_KOEO:
      switchFaultCode(true);
      break;
    case START_MESSAGE_TIMEOUT:
    case RESULT_FAULT_CODE:
      showMainMenu();
      break;
    case READING_FAULT_CODE:
    case RUNNING_KOEO:
      break;
  }
}

void onButtonDown() {
  switch(screenMode) {
    case MAIN_MENU:
      switchMainMenuMode(true);
      break;
    case RESULT_KOEO:
      switchFaultCode(false);
      break;
    case START_MESSAGE_TIMEOUT:
    case RESULT_FAULT_CODE:
      showMainMenu();
      break;
    case READING_FAULT_CODE:
    case RUNNING_KOEO:
      break;
  }
}

void onButtonSelect() {
  switch(screenMode) {
    case MAIN_MENU:
      selectMode();
      break;
    case START_MESSAGE_TIMEOUT:
    case RESULT_KOEO:
    case RESULT_FAULT_CODE:
      showMainMenu();
      break;
    case READING_FAULT_CODE:
    case RUNNING_KOEO:
      break;
  }
}

void showMainMenu() {
  eecIv.setMode(EecIv::OperationMode::READ_FAULTS);
  screenMode = MAIN_MENU;
  mainMenuMode = FAULT_CODE;
  drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Read Fault ", "Code Memory", "");
}

void switchMainMenuMode(bool down) {
  mainMenuMode = down ? (MAIN_MENU_MODE)((mainMenuMode+1)%NUM_MAIN_MENU_MODES) : (MAIN_MENU_MODE)((mainMenuMode+NUM_MAIN_MENU_MODES-1)%NUM_MAIN_MENU_MODES);
  switch (mainMenuMode) {
    case FAULT_CODE:
      drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Read Fault ", "Code Memory", "");
      break;
    case KOEO:
      drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Run System ", "Test", "");
      break;
#if true
    case LIVE_DATA:
      drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Live Data", "", "");
      break;
#endif
  }
}

void selectMode() {
  switch (mainMenuMode) {
    case FAULT_CODE:
      eecIv.setMode(EecIv::OperationMode::READ_FAULTS);
      eecIv.restartReading();
      screenMode = READING_FAULT_CODE;
      drawWaitingScreen();
      fault_codes_done = false;
      fault_code_pointer = 0;
      break;
    case KOEO:
      eecIv.setMode(EecIv::OperationMode::KOEO);
      eecIv.restartReading();
      screenMode = RUNNING_KOEO;
      drawWaitingScreen();
      fault_codes_done = false;
      fault_code_pointer = 0;
      break;
#if true
    case LIVE_DATA:
      eecIv.setMode(EecIv::OperationMode::LIVE_DATA);
      eecIv.restartReading();
      screenMode = READING_FAULT_CODE;
      drawWaitingScreen();
      break;
#endif
  }
}

void onStartMessageTimeout() {
  screenMode = RESULT_FAULT_CODE;
  drawMenuScreen(BACK_SIGN, NO_SIGN, NO_SIGN, "Timeout Error", "Is the igni-", "tion on?", "");
}

void switchFaultCode(bool down) {
  fault_code_display_pointer = down ? 
    (fault_code_display_pointer+koeo_i_max)%(koeo_i_max+1) : 
    (fault_code_display_pointer+1)%(koeo_i_max+1);

  char code_buf[16];

  sprintf(code_buf, "[%0d] %01X%02X", 
    fault_code_display_pointer+1,
    fault_codes[fault_code_display_pointer][1] & 0xF, 
    fault_codes[fault_code_display_pointer][0]);
  drawMenuScreen(BACK_SIGN, UP_SIGN, DOWN_SIGN, "Fault Code", code_buf, "", "");
}

void onFaultCodeRead(const uint8_t data[]) {
  if (fault_codes_done) {
    return;
  }

  Serial.println("onfaultcode");

  // just to be save here... 
  if (fault_code_pointer >= 12) {
    fault_code_pointer = 0;
  }

  memcpy(fault_codes[fault_code_pointer], data, 2);
  fault_code_pointer++;
}

void onFaultCodeFinished() {
  char code_buf[16];

  fault_codes_done = true;
  koeo_i_max = fault_code_pointer-1;
  
  sprintf(code_buf, "Fault codes found: %d", fault_code_pointer);
  Serial.println(code_buf);

  // no fault codes set
  if (fault_code_pointer == 0) {
    drawMenuScreen(BACK_SIGN, NO_SIGN, NO_SIGN, "Fault Code", "None found", "", "");
    screenMode = RESULT_FAULT_CODE;
    return;
  }

  //fault_code_pointer = 0;
  fault_code_display_pointer = 0;
  
  screenMode = RESULT_KOEO;
  sprintf(code_buf, "[%0d] %01X%02X", 
    fault_code_display_pointer+1, 
    fault_codes[fault_code_display_pointer][1] & 0xF, 
    fault_codes[fault_code_display_pointer][0]);

  drawMenuScreen(BACK_SIGN, UP_SIGN, DOWN_SIGN, "Fault Code", code_buf, "", "");
}

void onLiveData(const uint8_t data[]) {
  sprintf(liveDataBuf, "%01X%02X", data[1] & 0xF, data[0]);
  u8x8.clear();

  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(1, 3, liveDataBuf);
}