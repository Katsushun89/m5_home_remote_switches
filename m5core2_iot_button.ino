#include <M5Core2.h>
#include <LovyanGFX.hpp>
#include "ui_draw.h"

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

void setup(void)
{
  Serial.begin(115200);

  lcd.init();
  int lw = std::min(lcd.width(), lcd.height());

  center_px = lcd.width() >> 1;
  center_py = lcd.height() >> 1;
  zoom = (float)(std::min(lcd.width(), lcd.height())) / center_button_width;

  lcd.setPivot(center_px, center_py);
  canvas.setColorDepth(lgfx::palette_4bit);
  center_base.setColorDepth(lgfx::palette_4bit);
  center_button.setColorDepth(lgfx::palette_4bit);

  button_width = lcd.width() * 2 / 3;

  canvas.createSprite(lcd.width(), lcd.height());
  center_base.createSprite(button_width, button_width);
  center_button.createSprite(button_width, button_width);

  canvas.fillScreen(transpalette);
  center_base.fillScreen(transpalette);
  center_button.fillScreen(transpalette);

  canvas.setPaletteColor(1, 0, 0, 15);
  canvas.setPaletteColor(PALETTE_ORANGE, 255, 102, 0);
  //canvas.setPaletteColor(3, 255, 255, 191);
  canvas.setPaletteColor(3, lcd.color888(255, 51, 0));
  canvas.setPaletteColor(4, lcd.color888(255, 81, 0));

  center_base.setPaletteColor(1, 0, 0, 15);
  center_base.setPaletteColor(3, lcd.color888(255, 51, 0));
  center_base.setPaletteColor(4, lcd.color888(255, 81, 0));

  center_button.setPaletteColor(3, lcd.color888(255, 51, 0));
  center_button.setPaletteColor(4, lcd.color888(255, 81, 0));

  /*
  base  .createSprite(width, width);

  base.setFont(&fonts::Orbitron_Light_24);    // フォント種類を変更(盤の文字用)
//base.setFont(&fonts::Roboto_Thin_24);       // フォント種類を変更(盤の文字用)

  base.setTextDatum(lgfx::middle_center);

  base.fillCircle(halfwidth, halfwidth, halfwidth - 8, 1);
  base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 11, 135,  45, 3);
  base.fillArc(halfwidth, halfwidth, halfwidth - 20, halfwidth - 23,   2,  43, 2);
  base.fillArc(halfwidth, halfwidth, halfwidth - 20, halfwidth - 23, 317, 358, 2);
  base.setTextColor(3);

  for (int i = -5; i <= 25; ++i) {
    bool flg = 0 == (i % 5);      // ５目盛り毎フラグ
    if (flg) {
      // 大きい目盛り描画
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 24, 179.8 + i*9, 180.2 + i*9, 3);
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 20, 179.4 + i*9, 180.6 + i*9, 3);
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 14, 179   + i*9, 181   + i*9, 3);
      float rad = i * 9 * 0.0174532925;
      float ty = - sin(rad) * (halfwidth * 10 / 15);
      float tx = - cos(rad) * (halfwidth * 10 / 17);
      base.drawFloat((float)i/10, 1, halfwidth + tx, halfwidth + ty); // 数値描画
    } else {
      // 小さい目盛り描画
      base.fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 17, 179.5 + i*9, 180.5 + i*9, 3);
    }
  }
*/
  lcd.startWrite();
  //center_base.fillRect(0, 0, button_width, button_width, 1);
  //center_base.drawRect(0, 0, button_width, button_width, PALETTE_ORANGE);
  //center_base.pushRotateZoom(0, zoom, zoom, transpalette);    // 完了した盤をLCDに描画する
  drawCenterOFF();
  drawCenterBase();
  center_button.pushRotateZoom(0, zoom, zoom, transpalette);
  canvas.pushSprite(0, 0);
}

const uint32_t CENTER_ON_TIME = 1000 * 1;
const uint32_t CENTER_OFF_TIME = 500;
const uint32_t INVALID_DURATION = 1000 * 1;
bool is_switched_on_center = false;
bool is_in_transition_center_state = false;
bool is_once_released_after_switch_on = false;
uint32_t last_operation_time = 0;
uint32_t start_time_push_center = 0;
uint32_t keep_time_push_center = 0;
uint32_t invalid_time = 0;

/*
void draw(float value)
{
  //base.pushSprite(0, 0);  // 描画用バッファに盤の画像を上書き

  //float angle = 270 + value * 90.0;
  //int center = button_width >> 1;
  //canvas.fillCircle(center, center, 120, 3);
  //canvas.fillCircle(center, center, 80, 2);
  //canvas.fillCircle(center, center,  40, 1);
  //canvas.fillArc(center, center, center -  0, center - 20, 180, 0, PALETTE_ORANGE);
  //canvas.fillArc(center, center, center - 20, center - 40, 90, 45, 3);
  //canvas.fillArc(center, center, center - 40, center - 60, 90, 0, 4);
  //canvas.drawRect(0, 0, button_width, button_width, PALETTE_ORANGE);  // 矩形の外周
  //canvas.drawString("ON", 10, 20);

  drawCenterOFF();
  canvas.pushRotateZoom(0, zoom, zoom, transpalette);    // 完了した盤をLCDに描画する
  // if (value >= 1.5)
  //   lcd.fillCircle(lcd.width()>>1, (lcd.height()>>1) + width * 4/10, 5, 0x007FFFU);
}
*/

void drawCenterBase(void)
{
  int center = button_width >> 1;
  center_base.fillRect(0, 0, button_width, button_width, 1);
  center_base.drawRect(0, 0, button_width, button_width, PALETTE_ORANGE);
  center_base.pushRotateZoom(0, zoom, zoom, transpalette);    // 完了した盤をLCDに描画する
}

void drawCenterTransitionOff2On(uint32_t keep_push_time)
{
  int center = button_width >> 1;
  center_button.drawString("TRS", 10, 20);
  center_button.fillCircle(center, center, center-1, 1);

  uint32_t angle = uint32_t(float(keep_push_time) / float(getDecisionTime()) * 360.);
  if(angle > 360) angle = 360;
  Serial.printf("angle %d\n", angle);
  center_button.drawCircle(center, center, center-2, 3);
  center_button.drawCircle(center, center, center-22, 3);
  center_button.fillArc(center, center, center-2, center-22, 0, angle, 4);
}

void drawCenterTransitionOn2Off(uint32_t keep_push_time)
{
  int center = button_width >> 1;
  center_button.drawString("TRS", 10, 20);
  center_button.fillCircle(center, center, center-1, 1);

  uint32_t r = uint32_t(float(keep_push_time) / float(getDecisionTime()) * 20.);
  if(r > 20) r = 20;
  Serial.printf("r %d\n", r);
  center_button.drawCircle(center, center, center-2, 3);
  center_button.drawCircle(center, center, center-22, 3);
  center_button.fillArc(center, center, center-2-r, center-22, 0, 360, 4);
}

void drawCenterON(void)
{
  Serial.println("drawCenterOn\n");
  int center = button_width >> 1;
  center_button.drawString("ON ", 10, 20);
  center_button.fillCircle(center, center, center-1, 1);
  center_button.drawCircle(center, center, center-2, 3);
  center_button.drawCircle(center, center, center-22, 3);
  //center_button.drawArc(center, center, center-2, center-22, 0, 359, 4);
  center_button.fillArc(center, center, center-2, center-22, 0, 360, 4);
}

void drawCenterOFF(void)
{
  Serial.println("drawCenterOff\n");
  int center = button_width >> 1;
  center_button.drawString("OFF", 10, 20);
  center_button.fillCircle(center, center, center-1, 1);
  center_button.drawCircle(center, center, center-2, 3);
  center_button.drawCircle(center, center, center-22, 3);
  //center_button.drawArc(center, center, center-2, center-22, -10, 0, 3);
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

uint32_t getDecisionTime(void)
{
  if(is_switched_on_center){
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
    is_switched_on_center = !is_switched_on_center;
    is_in_transition_center_state = false;
    invalid_time = cur_time + INVALID_DURATION;
    is_once_released_after_switch_on = true;
    uidraw.pushEvent(collectButtonStatus());
    Serial.printf("is_switched_on_center %d\n", is_switched_on_center);
    return;
  }

  //Serial.printf("1:%d, %d\n", cur_time, keep_time_push_center);
  uidraw.pushEvent(collectButtonStatus());
  return;

}

ButtonStatus collectButtonStatus(void)
{
  ButtonStatus status;
  status.is_switched_on = is_switched_on_center;
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

void loop(void)
{
  TouchPoint_t pos= M5.Touch.getPressPoint();
  bool is_touch_pressed = false;
  if(M5.Touch.ispressed()) is_touch_pressed = true;

  judgeBottomButtons(pos, is_touch_pressed);
  judgeCenterButton(pos, is_touch_pressed);
  tryDrawCenter();
  delay(10);
}

