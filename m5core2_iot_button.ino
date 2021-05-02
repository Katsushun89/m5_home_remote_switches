#include <M5Core2.h>
#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "ui_draw.h"
#include "switches.h"
#include "config.h"

static LGFX lcd;
static LGFX_Sprite canvas(&lcd);
static LGFX_Sprite center_base(&canvas);
static LGFX_Sprite center_button(&canvas);
int center_px = 0;
int center_py = 0;
static int32_t center_button_width = 239;
static float zoom;
static auto transpalette = 0;
static int button_width;

const int PALETTE_ORANGE = 2;

UIDraw uidraw;
Switches switches;

FirebaseData fbdo1;
FirebaseData fbdo2;

#define CHILD_NUM (2)
String parentPath = "/switches";
String childPath[CHILD_NUM] = {"/CRAFTROOM","/3D PRINTER"};
size_t childPathSize = CHILD_NUM;

void setupWiFi()
{
  WiFi.begin(SSID, PASSWD);

  // Wait some time to connect to wifi
  for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
      Serial.print(".");
      delay(1000);
  }

  // Check if connected to wifi
  if(WiFi.status() != WL_CONNECTED) {
      Serial.println("No Wifi!");
      return;
  }

  Serial.println("Connected to Wifi, Connecting to server.");
}

void streamCallback(MultiPathStreamData stream)
{
  Serial.println();
  Serial.println("Stream Data1 available...");

  size_t numChild = sizeof(childPath)/sizeof(childPath[0]);

  for(size_t i = 0;i< numChild;i++){
    if (stream.get(childPath[i])){
      Serial.println("path: " + stream.dataPath + ", type: " + stream.type + ", value: " + stream.value);
      bool power = false;
      if(stream.value.indexOf("true") >= 0){
        power = true;
      }
      updateFirebasePowerStatus(stream.dataPath, power);
    }
  }

  Serial.println();
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
  {
    Serial.println();
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
}

void setupFirebase(void)
{
  Firebase.begin(FIREBASE_DATABASE_URL, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginMultiPathStream(fbdo1, parentPath, childPath, childPathSize))
  {
    Serial.println("------------------------------------");
    Serial.println("Can't begin stream connection...");
    Serial.println("REASON: " + fbdo1.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }

  //Set the reserved size of stack memory in bytes for internal stream callback processing RTOS task.
  //8192 is the minimum size.
  Firebase.setMultiPathStreamCallback(fbdo1, streamCallback, streamTimeoutCallback, 8192);
}

void updateFirebasePowerStatus(String path, bool power)
{
  switches.updatePowerStatus(path, power);
  updateDrawingCenter();
}

void setup(void)
{
  Serial.begin(115200);

  lcd.init();
  int lw = std::min(lcd.width(), lcd.height());

  center_px = lcd.width() >> 1;
  center_py = lcd.height() >> 1;
  zoom = (float)(std::min(lcd.width(), lcd.height())) / center_button_width;

  lcd.setPivot(center_px, center_py);
  canvas.setColorDepth(lgfx::palette_2bit);
  center_base.setColorDepth(lgfx::palette_2bit);
  center_button.setColorDepth(lgfx::palette_2bit);

  button_width = lcd.width() * 7 / 10;

  canvas.createSprite(lcd.width(), lcd.height());
  center_base.createSprite(button_width, button_width);
  center_button.createSprite(button_width, button_width);

  canvas.fillScreen(transpalette);
  center_base.fillScreen(transpalette);
  center_button.fillScreen(transpalette);

  canvas.setPaletteColor(1, 0, 0, 15);
  canvas.setPaletteColor(PALETTE_ORANGE, 255, 102, 0);
  //canvas.setPaletteColor(3, 255, 255, 191);
  //canvas.setPaletteColor(3, lcd.color888(255, 51, 0));
  //canvas.setPaletteColor(4, lcd.color888(255, 81, 0));

  center_base.setPaletteColor(1, 0, 0, 15);
  //center_base.setPaletteColor(3, lcd.color888(255, 51, 0));
  //center_base.setPaletteColor(4, lcd.color888(255, 81, 0));

  center_button.setPaletteColor(PALETTE_ORANGE, lcd.color888(255, 51, 0));
  //center_button.setPaletteColor(3, lcd.color888(255, 81, 0));

  center_button.setTextFont(4);
  center_button.setTextDatum(lgfx::middle_center);

  lcd.startWrite();
  drawLRButton();
  drawCenterOFF();
  drawCenterBase();
  center_button.pushRotateZoom(0, zoom, zoom, transpalette);
  canvas.pushSprite(0, 0);

  switches.setFirebasePath(STUDIO_LIGHT, childPath[0]);
  switches.setFirebasePath(PRINTER_3D, childPath[1]);

  setupWiFi();
  setupFirebase();
}

const uint32_t CENTER_ON_TIME = 1200;
const uint32_t CENTER_OFF_TIME = 650;
const uint32_t INVALID_DURATION = 1000 * 1;
bool is_in_transition_center_state = false;
bool is_once_released_after_switch_on = false;
uint32_t last_operation_time = 0;
uint32_t start_time_push_center = 0;
uint32_t keep_time_push_center = 0;
uint32_t invalid_time = 0;

const uint32_t RING_INSIDE_WIDTH = 20;
const uint32_t RING_OUTSIDE_WIDTH = 6;
const uint32_t RING_TOTAL_WIDTH = RING_OUTSIDE_WIDTH+RING_INSIDE_WIDTH;
const uint8_t RING_INSIDE_DIV = 6;

void drawLRButton(void)
{
  const int inside_x = 30;
  const int outside_x = 5;
  const int top_y = (lcd.height() >> 1) - 20;
  const int mdl_y = (lcd.height() >> 1) - 0;
  const int btm_y = (lcd.height() >> 1) + 20;
  canvas.fillTriangle(inside_x, top_y, inside_x, btm_y, outside_x, mdl_y, PALETTE_ORANGE);//left
  canvas.fillTriangle(lcd.width()-inside_x, top_y, lcd.width()-inside_x, btm_y, lcd.width()-outside_x, mdl_y, PALETTE_ORANGE);//right
}

void drawCenterBase(void)
{
  int center = button_width >> 1;
  center_base.fillRect(0, 0, button_width, button_width, 1);
  center_base.pushRotateZoom(0, zoom, zoom, transpalette);
}

void drawCenterTransitionOff2On(uint32_t keep_push_time)
{
  int center = button_width >> 1;
  center_button.fillCircle(center, center, center-1, 1);//clear
  center_button.drawCircle(center, center, center-2-RING_OUTSIDE_WIDTH, PALETTE_ORANGE);//outside
  center_button.drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);//inside

  const uint32_t time_point1 = getDecisionTime() * 1 / 3;
  const uint32_t time_point2 = getDecisionTime() * 2 / 3;
  if(keep_push_time < time_point1){
    uint32_t r = uint32_t(float(keep_push_time) / float(time_point1) * float(RING_OUTSIDE_WIDTH));
    if(r > RING_OUTSIDE_WIDTH) r = RING_OUTSIDE_WIDTH;
    center_button.fillArc(center, center, center-2+r-RING_OUTSIDE_WIDTH, center-2-RING_OUTSIDE_WIDTH, 0, 20, PALETTE_ORANGE);
    center_button.fillArc(center, center, center-2+r-RING_OUTSIDE_WIDTH, center-2-RING_OUTSIDE_WIDTH, 180, 180+20, PALETTE_ORANGE);
    for(int i = 0; i < RING_INSIDE_DIV; i++){
      center_button.fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
    }
  }else if(keep_push_time < time_point2 && keep_push_time >= time_point1){
    uint32_t angle = uint32_t(float(keep_push_time-time_point1) / float(time_point2-time_point1) * 180.);
    if(angle > 180) angle = 180;
    center_button.fillArc(center, center, center-2, center-2-RING_OUTSIDE_WIDTH, 0, angle, PALETTE_ORANGE);
    center_button.fillArc(center, center, center-2, center-2-RING_OUTSIDE_WIDTH, 180, 180+angle, PALETTE_ORANGE);
    for(int i = 0; i < RING_INSIDE_DIV; i++){
      center_button.fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
    }
  }else if(keep_push_time >= time_point2){
    center_button.fillArc(center, center, center-2, center-2-RING_OUTSIDE_WIDTH, 0, 360, PALETTE_ORANGE);
    for(int i = 0; i < RING_INSIDE_DIV; i++){
      center_button.fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
    }
    uint32_t r = uint32_t(float(keep_push_time-time_point2) / float(getDecisionTime()-time_point2) * RING_INSIDE_WIDTH);
    if(r > RING_INSIDE_WIDTH) r = RING_INSIDE_WIDTH;
    center_button.fillArc(center, center, center-2+r-RING_TOTAL_WIDTH, center-2-RING_TOTAL_WIDTH, 0, 360, PALETTE_ORANGE);
  }
}

void drawCenterTransitionOn2Off(uint32_t keep_push_time)
{
  int center = button_width >> 1;
  center_button.fillCircle(center, center, center-1, 1);//clear
  center_button.drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);//inside
  for(int i = 0; i < RING_INSIDE_DIV; i++){
    center_button.fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
  }
  //outside
  //uint32_t r1 = uint32_t(float(keep_push_time) / float(getDecisionTime()) * float(RING_OUTSIDE_WIDTH));
  //if(r1 > RING_OUTSIDE_WIDTH) r1 = RING_OUTSIDE_WIDTH;
  center_button.drawCircle(center, center, center-2-RING_OUTSIDE_WIDTH, PALETTE_ORANGE);//outside
  //fill
  uint32_t r2 = uint32_t(float(keep_push_time) / float(getDecisionTime()) * float(RING_TOTAL_WIDTH));
  if(r2 > RING_TOTAL_WIDTH) r2 = RING_TOTAL_WIDTH;
  center_button.fillArc(center, center, center-2-r2, center-2-RING_TOTAL_WIDTH, 0, 360, PALETTE_ORANGE);
}

void updateButtonStr(bool onOff)
{
  int x = center_button.getPivotX() - 55;
  int y = center_button.getPivotY();
  center_button.setCursor(x, y);
  center_button.setTextColor(PALETTE_ORANGE);
  center_button.setTextSize(0.75);

  center_button.printf("%s", switches.getStrCurrentSwitch().c_str());
}

void drawCenterON(void)
{
  Serial.println("drawCenterOn\n");
  int center = button_width >> 1;
  center_button.fillCircle(center, center, center-1, 1);
  center_button.drawCircle(center, center, center-2, PALETTE_ORANGE);
  center_button.drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);
  center_button.fillArc(center, center, center-2, center-2-RING_TOTAL_WIDTH, 0, 360, PALETTE_ORANGE);
  updateButtonStr(true);
}

void drawCenterOFF(void)
{
  Serial.println("drawCenterOff\n");
  int center = button_width >> 1;
  center_button.fillCircle(center, center, center-1, 1);
  center_button.drawCircle(center, center, center-2-RING_OUTSIDE_WIDTH, PALETTE_ORANGE);
  center_button.drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);
  for(int i = 0; i < RING_INSIDE_DIV; i++){
    center_button.fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
  }
  updateButtonStr(false);
}

void drawCenter(ButtonStatus &status)
{
  if(status.is_in_transition){
    if(status.is_switched_on){
      drawCenterTransitionOn2Off(status.keep_push_time);
    }else{
      drawCenterTransitionOff2On(status.keep_push_time);
    }
  }else{
    if(status.is_switched_on){
      drawCenterON();
    }else{
      drawCenterOFF();
    }
  }
  drawCenterBase();
  center_button.pushRotateZoom(0, zoom, zoom, transpalette);
  canvas.pushSprite(0, 0);
}

void tryDrawCenter(void)
{
  ButtonStatus status;
  if(uidraw.popEvent(status)){
    drawCenter(status);
  }
}

void updateDrawingCenter(void)
{
  ButtonStatus status;
  status.is_switched_on = switches.isSwitchedOnCurrentSwitch();
  drawCenter(status);
}

uint32_t getDecisionTime(void)
{
  if(switches.isSwitchedOnCurrentSwitch()){
    return CENTER_OFF_TIME;
  }else{
    return CENTER_ON_TIME;
  }
}

void keepTouchCenterButton(void)
{
  uint32_t cur_time = millis();

  if(cur_time <= invalid_time){
    //Disable INVALID_DURATION after the button state is switched.
    //Serial.printf("invalid center after switched %d %d\n", cur_time, invalid_time);
    return;
  }

  //Once the switch is turned on, it will not turn on again until you release it.
  if(is_once_released_after_switch_on){
    //Serial.printf("invalid until once released\n");
    return;
  }

  if(!is_in_transition_center_state){
    is_in_transition_center_state = true;
    start_time_push_center = cur_time;
    last_operation_time = cur_time;

    //Serial.printf("start transition %d\n", cur_time);
    return;
  }

  int32_t diff = cur_time - last_operation_time;
  keep_time_push_center += diff;
  last_operation_time = cur_time;

  //switch on
  if(keep_time_push_center >= getDecisionTime()){
    keep_time_push_center = 0;
    bool is_switched_on = switches.toggleSwitch();

    //sync firebase rtdb
    //Firebase.setBool(switches.getFirebasePathCurrentSwitch().c_str(), is_switched_on);
    bool ret = Firebase.setBool(fbdo2, parentPath + switches.getFirebasePathCurrentSwitch() + "/power", is_switched_on);
    if(!ret){
      Serial.printf("setBool error %s\n", fbdo2.errorReason().c_str());
    }

    is_in_transition_center_state = false;
    invalid_time = cur_time + INVALID_DURATION;
    is_once_released_after_switch_on = true;
    uidraw.pushEvent(collectButtonStatus());
    return;
  }

  //Serial.printf("1:%d, %d\n", cur_time, keep_time_push_center);
  uidraw.pushEvent(collectButtonStatus());
  return;

}

ButtonStatus collectButtonStatus(void)
{
  ButtonStatus status;
  status.is_switched_on = switches.isSwitchedOnCurrentSwitch();
  status.is_in_transition = is_in_transition_center_state;
  status.keep_push_time = keep_time_push_center;
  return status;
}

void keepReleaseCenterButton(void)
{
  if(is_once_released_after_switch_on) is_once_released_after_switch_on = false;

  if(is_in_transition_center_state){
    resetTranstionPrams();
    uidraw.pushEvent(collectButtonStatus());
  }
}

void judgeCenterButton(TouchPoint_t pos, bool is_touch_pressed)
{
  if(is_touch_pressed){
    if(pos.y >= center_py - center_button_width / 2 &&
       pos.y <  center_py + center_button_width / 2 ){
        if(pos.x >= center_px - center_button_width / 2 &&
          pos.x <  center_px + center_button_width / 2 ){
          keepTouchCenterButton();
          return;
        }
    }
  }
  keepReleaseCenterButton();
  return;
}

void resetTranstionPrams(void)
{
  is_in_transition_center_state = false;
  is_once_released_after_switch_on = false;
  start_time_push_center = 0;
  keep_time_push_center = 0;
  invalid_time = 0;
  Serial.println("resetTranstionPrams");
}

void judgeBottomButtons(TouchPoint_t pos, bool is_touch_pressed)
{
  static bool is_button_pressed = false;
  if(!is_touch_pressed) is_button_pressed = false;

  if(!is_button_pressed){
    if(pos.y > 240){
      if(pos.x < 120){//btnA
        is_button_pressed = true;
        resetTranstionPrams();
      }
      else if(pos.x > 240){ //btnC
        is_button_pressed = true;
      }
      else if(pos.x >= 180 && pos.x <= 210){ //btnB
        is_button_pressed = true;
      }
    }
  }
}

void judgeLRButton(TouchPoint_t pos, bool is_touch_pressed)
{
  static bool is_button_pressed = false;
  if(!is_touch_pressed) is_button_pressed = false;

  if(!is_button_pressed){
    if(pos.y > (lcd.height() >> 1) - 40 &&
       pos.y < (lcd.height() >> 1) + 40 ){
      if(pos.x < 50){//L
        is_button_pressed = true;
        Serial.println("push L");
        switches.movedown();
        updateDrawingCenter();
      }
      else if(pos.x > lcd.width() - 50){ //R
        is_button_pressed = true;
        Serial.println("push R");
        switches.moveup();
        updateDrawingCenter();
      }
    }
  }
}



void loop(void)
{
  TouchPoint_t pos= M5.Touch.getPressPoint();
  bool is_touch_pressed = false;
  if(M5.Touch.ispressed()) is_touch_pressed = true;

  judgeBottomButtons(pos, is_touch_pressed);
  judgeCenterButton(pos, is_touch_pressed);
  judgeLRButton(pos, is_touch_pressed);
  tryDrawCenter();
  delay(10);
}

