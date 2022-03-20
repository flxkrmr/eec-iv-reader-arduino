#include <Arduino.h>

#include <menu.h>
#include <menuIO/u8g2Out.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#include "EecIv.h"

#include <Wire.h>

// Pins RS485
static const int DI=3;
static const int RE=6;
static const int RO=2;

// Pins Buttons
static const int BTN_1 = 7;
static const int BTN_2 = 8;
static const int BTN_3 = 9;

#define fontName u8g2_font_7x13_mf
#define fontX 7
#define fontY 16
#define offsetX 0
#define offsetY 3
#define U8_Width 128
#define U8_Height 64
#define USE_HWI2C

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0,  U8X8_PIN_NONE);

const colorDef<uint8_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

result actionCheckFaultCode();

MENU(mainMenu,"EEC IV Reader",doNothing,noEvent,wrapStyle
  ,OP("Check Fault Code",actionCheckFaultCode,enterEvent)
  ,OP("Run KOEO/KOER",doNothing,enterEvent)
  ,OP("Read Live Data",doNothing,noEvent)
);

#define MAX_DEPTH 2

keyMap encBtn_map[]={
  {BTN_1,defaultNavCodes[upCmd].ch},
  {BTN_2,defaultNavCodes[enterCmd].ch},
  {BTN_3,defaultNavCodes[downCmd].ch}
};

keyIn<3> encButton(encBtn_map);

serialIn serial(Serial);
MENU_INPUTS(in,&encButton, &serial);

MENU_OUTPUTS(out,MAX_DEPTH
  ,U8G2_OUT(u8g2,colors,fontX,fontY,offsetX,offsetY,{0,0,U8_Width/fontX,U8_Height/fontY})
  ,NONE
);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void serialPrint(char message[]) {
  Serial.println(message);
}

EecIv eecIv = EecIv(DI, RO, RE, serialPrint);

int mode = 0;

result idle(menuOut& o,idleEvent e) {
  o.clear();
  switch(e) {
    case idleStart:
      o.println("suspending menu!");
      break;
    case idling:
      o.println("suspended...");
      break;
    case idleEnd:
      o.println("resuming menu.");
      break;
  }
  return proceed;
}

void setup() {
  Serial.begin(19200);
  encButton.begin();
  Serial.println("### EEC IV Reader ###");

  Wire.begin();
  u8g2.begin();
  u8g2.setFont(fontName);
  mainMenu[2].enabled=disabledStatus; // live data disabled
  nav.idleTask=idle;//point a function to be used when menu is suspended

  eecIv.setup();

  eecIv.setModeFaultCode();
}

void loop() {
  nav.doInput();
  if (nav.changed(0)) {
    u8g2.firstPage();
    do nav.doOutput(); while(u8g2.nextPage());
  }

  eecIv.mainLoop();
}


result actionCheckFaultCode() {
  eecIv.setModeFaultCode();
  eecIv.restartReading();
  return proceed;
}

void restartButtonCallback() {
  eecIv.restartReading();
}

void modeButtonCallback() {
  mode++;
  if (mode > 2) 
    mode = 0;

  switch(mode) {
    case 0:
      //eecIv.setModeFaultCode();
      break;
    case 1:
      //eecIv.setModeKoeo();
      break;
    case 2:
      //eecIv.setModeLiveData();
      break;
  }
}