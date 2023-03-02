#include <Arduino.h>
#include <U8x8lib.h>
#include <OneButton.h>

#include "EecIv.h"
#include "VoltageReader.h"

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

#define VOL A0


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

// enable voltage monitor
//#define VOLTAGE_MONITOR

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

void drawVoltageScreen(double voltage);
void drawWelcomeScreen();
void drawMenuScreen(const char selectSign, const char upSign, const char downSign, const char heading[], const char bodyLine1[], const char bodyLine2[], const char bodyLine3[]);


EecIv eecIv (DI, RO, RE);
VoltageReader voltageReader(VOL);

#ifdef VOLTAGE_MONITOR
#define NUM_MAIN_MENU_MODES 3
enum MAIN_MENU_MODE {
  FAULT_CODE,
  KOEO,
  VOLTAGE
};
#else
#define NUM_MAIN_MENU_MODES 3
enum MAIN_MENU_MODE {
  FAULT_CODE,
  KOEO,
  LIVE_DATA
};
#endif


MAIN_MENU_MODE mainMenuMode = FAULT_CODE;

enum SCREEN_MODE {
  MAIN_MENU,
  START_MESSAGE_TIMEOUT,
  READING_FAULT_CODE,
  RESULT_FAULT_CODE,
  RUNNING_KOEO,
  RESULT_KOEO,
  SHOW_VOLTAGE
};
SCREEN_MODE screenMode = MAIN_MENU;


char koeo_codes[12][4]; // all koeo codes, maximum 12 with each lengh 4
uint8_t koeo_i = 0; // index for reading koeo
uint8_t koeo_code = 0; // index for showing koeo
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

  voltageReader.onVoltage = &drawVoltageScreen;

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

void drawVoltageScreen(double voltage) {
  char str[100];
  const char *tmpSign = (voltage < 0) ? "-" : "";
  float tmpVal = (voltage < 0) ? -voltage : voltage;

  int tmpInt1 = tmpVal;                  // Get the integer (678).
  float tmpFrac = tmpVal - tmpInt1;      // Get fraction (0.0123).
  int tmpInt2 = trunc(tmpFrac * 10000);  // Turn into integer (123).

  // Print as parts, note that you need 0-padding for fractional bit.

  sprintf(str, "%s%d.%04d V", tmpSign, tmpInt1, tmpInt2);

  u8x8.setFont(u8x8_font_8x13B_1x2_f);
  u8x8.drawString(1, 0, "Voltage");
  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(0, 2, "               ");
  u8x8.drawString(0, 2, str);

  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.drawGlyph(NUM_COLUMN - 1, NUM_ROW/2 - 1, BACK_SIGN);
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
  if (screenMode == SHOW_VOLTAGE) {
    voltageReader.loop();
  }
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
    case SHOW_VOLTAGE:
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
    case SHOW_VOLTAGE:
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
    case SHOW_VOLTAGE:
      showMainMenu();
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
#ifdef VOLTAGE_MONITOR
    case VOLTAGE:
      drawMenuScreen(SELECT_SIGN, UP_SIGN, DOWN_SIGN, HEADING_SELECT, "Measure", "Voltage", "");
      break;
#endif
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
      break;
    case KOEO:
      eecIv.setMode(EecIv::OperationMode::KOEO);
      eecIv.restartReading();
      screenMode = RUNNING_KOEO;
      drawWaitingScreen();
      break;
#if true
    case LIVE_DATA:
      eecIv.setMode(EecIv::OperationMode::LIVE_DATA);
      eecIv.restartReading();
      screenMode = READING_FAULT_CODE;
      drawWaitingScreen();
      break;
#endif
#ifdef VOLTAGE_MONITOR
    case VOLTAGE:
      screenMode = SHOW_VOLTAGE;  
      u8x8.clear();
      break;
#endif
  }
}

void onStartMessageTimeout() {
  screenMode = RESULT_FAULT_CODE;
  drawMenuScreen(BACK_SIGN, NO_SIGN, NO_SIGN, "Timeout Error", "Is the igni-", "tion on?", "");
}

void switchFaultCode(bool down) {
  koeo_code = down ? (koeo_code+koeo_i_max)%(koeo_i_max+1) : (koeo_code+1)%(koeo_i_max+1);
  char code_buf[16];
  sprintf(code_buf, "[%0d] %s", koeo_code+1, koeo_codes[koeo_code]);
  drawMenuScreen(BACK_SIGN, UP_SIGN, DOWN_SIGN, "Fault Code", code_buf, "", "");
}

void onFaultCodeRead(const uint8_t data[]) {
  sprintf(koeo_codes[koeo_i], "%01X%02X", data[1] & 0xF, data[0]);
  koeo_i++;
}

void onFaultCodeFinished() {
  char code_buf[16];

  koeo_i_max = koeo_i-1;
  
  sprintf(code_buf, "\nFault codes found: %d", koeo_i);
  Serial.println(code_buf);

  koeo_i = 0;
  koeo_code = 0;

  // no fault codes set
  if (koeo_i_max == -1) {
    drawMenuScreen(BACK_SIGN, NO_SIGN, NO_SIGN, "Fault Code", "None found", "", "");
    screenMode = RESULT_FAULT_CODE;
    return;
  }
  
  screenMode = RESULT_KOEO;
  sprintf(code_buf, "[%0d] %s", koeo_code+1, koeo_codes[koeo_code]);
  drawMenuScreen(BACK_SIGN, UP_SIGN, DOWN_SIGN, "Fault Code", code_buf, "", "");
}

void onLiveData(const uint8_t data[]) {
  sprintf(liveDataBuf, "%01X%02X", data[1] & 0xF, data[0]);
  u8x8.clear();

  u8x8.setFont(u8x8_font_8x13_1x2_f);
  u8x8.drawString(1, 3, liveDataBuf);
}