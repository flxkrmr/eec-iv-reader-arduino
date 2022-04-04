#include <Arduino.h>

#include <menu.h>
#include <menuIO/U8x8Out.h>
#include <U8x8lib.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <EasyButton.h>

using namespace Menu;

#include "EecIv.h"

#include <Wire.h>

// Pins RS485
static const int DI=3;
static const int RE=6;
static const int RO=2;

// Pins Buttons
#define BTN_1 7
#define BTN_2 8
#define BTN_3 9


//EasyButton button1(BTN_1);
EasyButton button2(BTN_2);
//EasyButton button3(BTN_3);

#define fontName u8g2_font_7x13_mf
#define fontX 7
#define fontY 16
#define offsetX 0
#define offsetY 3
#define U8_Width 128
#define U8_Height 64
#define USE_HWI2C

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

const colorDef<uint8_t> colors[6] MEMMODE={
  {{0,0},{0,1,1}},//bgColor
  {{1,1},{1,0,0}},//fgColor
  {{1,1},{1,0,0}},//valColor
  {{1,1},{1,0,0}},//unitColor
  {{0,1},{0,0,1}},//cursorColor
  {{1,1},{1,0,0}},//titleColor
};

result actionCheckFaultCode();
result actionRunKoeo();

MENU(mainMenu,"EEC IV Reader",doNothing,noEvent,wrapStyle
  ,OP("Read Fault Code",actionCheckFaultCode,enterEvent)
  ,OP("Run KOEO/KOER",actionRunKoeo,enterEvent)
  ,OP("Read Live Data",doNothing,noEvent)
);

#define MAX_DEPTH 1

keyMap encBtn_map[]={
  {BTN_1,defaultNavCodes[upCmd].ch},
  {BTN_2,defaultNavCodes[enterCmd].ch},
  {BTN_3,defaultNavCodes[downCmd].ch}
};

keyIn<3> encButton(encBtn_map);

serialIn serial(Serial);
MENU_INPUTS(in,&encButton, &serial);

idx_t u8x8_tops[MAX_DEPTH];
PANELS(u8x8Panels,{0,0,U8_Width/8,U8_Height/8});
U8x8Out u8x8Out(u8x8,u8x8_tops,u8x8Panels);

menuOut* const outputs[] MEMMODE={&u8x8Out};
outputsList out(outputs,1);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);

void serialPrint(char message[]) {
  Serial.println(message);
}

void onFaultCodeFinished(char message[]);

EecIv eecIv = EecIv(DI, RO, RE);

int mode = 0;
bool eecIvBusy = 0;

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
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  mainMenu[2].enabled=disabledStatus; // live data disabled
  //nav.idleTask=idle;//point a function to be used when menu is suspended

  
  //button1.begin();
  //button1.onPressed(exitResultView);
  button2.begin();
  button2.onPressed(exitResultView);
  //button3.begin();
  //button3.onPressed(exitResultView);

  eecIv.debugPrint = &serialPrint;
  eecIv.onFaultCodeFinished = &onFaultCodeFinished;
  eecIv.onKoeoFinished = &onKoeoFinished;
  eecIv.setup();

  eecIv.setModeFaultCode();
}

void loop() {
  if (!eecIvBusy) {
    nav.poll();
  } else {
    //button1.read();
    button2.read();
    //button3.read();
  }

  eecIv.mainLoop();
}


result actionCheckFaultCode() {
  u8x8.clear();
  u8x8.drawString(0,0,"Reading");
  u8x8.drawString(0,1,"Fault Code...");
  
  eecIv.setModeFaultCode();
  eecIv.restartReading();
  eecIvBusy = 1;

  return proceed;
}


result actionRunKoeo() {
  u8x8.clear();
  u8x8.drawString(0,0,"Running");
  u8x8.drawString(0,1,"KOEO/KOER...");
  
  eecIv.setModeKoeo();
  eecIv.restartReading();
  eecIvBusy = 1;

  return proceed;
}

void exitResultView() {
  eecIvBusy = 0;
  delay(250); // don't trigger menu directly
  nav.refresh();
}

void onKoeoFinished(char message[]) {
  u8x8.clear();
  u8x8.drawString(0,0,"KOEO Codes:");
  u8x8.drawString(0,1,message);
}

void onFaultCodeFinished(char message[]) {
  u8x8.clear();
  u8x8.drawString(0,0,"Fault Code:");
  u8x8.drawString(0,1,message);
}