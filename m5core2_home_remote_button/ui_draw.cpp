//#include <LovyanGFX.hpp>
#include "ui_draw.h"

UIDraw::UIDraw()
{
    lcd = new LGFX();
    canvas = new LGFX_Sprite(lcd);
    center_base = new LGFX_Sprite(canvas);
    center_button = new LGFX_Sprite(canvas);

    while (!event_queue.empty()) {
        event_queue.pop();
    }
}

void UIDraw::pushEvent(ButtonStatus status)
{
    event_queue.push(status);
}

bool UIDraw::popEvent(ButtonStatus &status)
{
    if(!event_queue.empty()) {
        status = event_queue.front();
        event_queue.pop();
        return true;
    }
    return false;
}

void UIDraw::setup(void)
{
    lcd->init();

    lcd_width = lcd->width();
    center_px = lcd->width() >> 1;
    center_py = lcd->height() >> 1;
    zoom = (float)(std::min(lcd->width(), lcd->height())) / CENTER_BUTTON_WIDTH;

    lcd->setPivot(center_px, center_py);

    canvas->setColorDepth(lgfx::palette_2bit);
    center_base->setColorDepth(lgfx::palette_2bit);
    center_button->setColorDepth(lgfx::palette_2bit);

    center_button_width = lcd->width() * 7 / 10;

    canvas->createSprite(lcd->width(), lcd->height());
    center_base->createSprite(center_button_width, center_button_width);
    center_button->createSprite(center_button_width, center_button_width);

    auto transpalette = 0;
    canvas->fillScreen(transpalette);
    center_base->fillScreen(transpalette);
    center_button->fillScreen(transpalette);

    canvas->setPaletteColor(PALETTE_BLACK, lcd->color888(0, 0, 15));
    canvas->setPaletteColor(PALETTE_ORANGE, lcd->color888(255, 81, 0));
    center_base->setPaletteColor(PALETTE_BLACK, lcd->color888(0, 0, 15));
    center_button->setPaletteColor(PALETTE_ORANGE, lcd->color888(255, 81, 0));

    center_button->setTextFont(4);
    center_button->setTextDatum(lgfx::middle_center);

    lcd->startWrite();
    drawLRButton();
    //drawCenterOFF(String(""));
    drawCenterBase();
    center_button->pushRotateZoom(0, zoom, zoom, transpalette);
    canvas->pushSprite(0, 0);
}

void UIDraw::drawLRButton(void)
{
    const int inside_x = 30;
    const int outside_x = 5;
    const int top_y = (lcd->height() >> 1) - 20;
    const int mdl_y = (lcd->height() >> 1) - 0;
    const int btm_y = (lcd->height() >> 1) + 20;
    canvas->fillTriangle(inside_x, top_y, inside_x, btm_y, outside_x, mdl_y, PALETTE_ORANGE);//left
    canvas->fillTriangle(lcd->width()-inside_x, top_y, lcd->width()-inside_x, btm_y, lcd->width()-outside_x, mdl_y, PALETTE_ORANGE);//right
}

void UIDraw::drawCenterBase(void)
{
    auto transpalette = 0;
    center_base->fillRect(0, 0, center_button_width, center_button_width, PALETTE_BLACK);
    center_base->pushRotateZoom(0, zoom, zoom, transpalette);
}

void UIDraw::drawCenterTransitionOff2On(uint32_t keep_push_time, uint32_t decision_time)
{
    int center = center_button_width >> 1;
    center_button->fillCircle(center, center, center-1, 1);//clear
    center_button->drawCircle(center, center, center-2-RING_OUTSIDE_WIDTH, PALETTE_ORANGE);//outside
    center_button->drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);//inside

    const uint32_t time_point1 = decision_time * 1 / 3;
    const uint32_t time_point2 = decision_time * 2 / 3;
    if(keep_push_time < time_point1){
        uint32_t r = uint32_t(float(keep_push_time) / float(time_point1) * float(RING_OUTSIDE_WIDTH));
        if(r > RING_OUTSIDE_WIDTH) r = RING_OUTSIDE_WIDTH;
        center_button->fillArc(center, center, center-2+r-RING_OUTSIDE_WIDTH, center-2-RING_OUTSIDE_WIDTH, 0, 20, PALETTE_ORANGE);
        center_button->fillArc(center, center, center-2+r-RING_OUTSIDE_WIDTH, center-2-RING_OUTSIDE_WIDTH, 180, 180+20, PALETTE_ORANGE);
        for(int i = 0; i < RING_INSIDE_DIV; i++){
            center_button->fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
        }
    }else if(keep_push_time < time_point2 && keep_push_time >= time_point1){
        uint32_t angle = uint32_t(float(keep_push_time-time_point1) / float(time_point2-time_point1) * 180.);
        if(angle > 180) angle = 180;
        center_button->fillArc(center, center, center-2, center-2-RING_OUTSIDE_WIDTH, 0, angle, PALETTE_ORANGE);
        center_button->fillArc(center, center, center-2, center-2-RING_OUTSIDE_WIDTH, 180, 180+angle, PALETTE_ORANGE);
        for(int i = 0; i < RING_INSIDE_DIV; i++){
            center_button->fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
        }
    }else if(keep_push_time >= time_point2){
        center_button->fillArc(center, center, center-2, center-2-RING_OUTSIDE_WIDTH, 0, 360, PALETTE_ORANGE);
        for(int i = 0; i < RING_INSIDE_DIV; i++){
            center_button->fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
        }
        uint32_t r = uint32_t(float(keep_push_time-time_point2) / float(decision_time-time_point2) * RING_INSIDE_WIDTH);
        if(r > RING_INSIDE_WIDTH) r = RING_INSIDE_WIDTH;
        center_button->fillArc(center, center, center-2+r-RING_TOTAL_WIDTH, center-2-RING_TOTAL_WIDTH, 0, 360, PALETTE_ORANGE);
    }
}

void UIDraw::drawCenterTransitionOn2Off(uint32_t keep_push_time, uint32_t decision_time)
{
    int center = center_button_width >> 1;
    center_button->fillCircle(center, center, center-1, 1);//clear
    center_button->drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);//inside
    for(int i = 0; i < RING_INSIDE_DIV; i++){
        center_button->fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
    }
    //outside
    //uint32_t r1 = uint32_t(float(keep_push_time) / float(decision_time) * float(RING_OUTSIDE_WIDTH));
    //if(r1 > RING_OUTSIDE_WIDTH) r1 = RING_OUTSIDE_WIDTH;
    center_button->drawCircle(center, center, center-2-RING_OUTSIDE_WIDTH, PALETTE_ORANGE);//outside
    //fill
    uint32_t r2 = uint32_t(float(keep_push_time) / float(decision_time) * float(RING_TOTAL_WIDTH));
    if(r2 > RING_TOTAL_WIDTH) r2 = RING_TOTAL_WIDTH;
    center_button->fillArc(center, center, center-2-r2, center-2-RING_TOTAL_WIDTH, 0, 360, PALETTE_ORANGE);
}

void UIDraw::updateButtonStr(bool onOff, String current_switch_str)
{
    int x = center_button->getPivotX() - 55;
    int y = center_button->getPivotY();
    center_button->setCursor(x, y);
    center_button->setTextColor(PALETTE_ORANGE);
    center_button->setTextSize(0.75);

    center_button->printf("%s", current_switch_str.c_str());
}

void UIDraw::drawCenterON(String current_switch_str)
{
    Serial.println("drawCenterOn\n");
    int center = center_button_width >> 1;
    center_button->fillCircle(center, center, center-1, 1);
    center_button->drawCircle(center, center, center-2, PALETTE_ORANGE);
    center_button->drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);
    center_button->fillArc(center, center, center-2, center-2-RING_TOTAL_WIDTH, 0, 360, PALETTE_ORANGE);
    updateButtonStr(true, current_switch_str);
}

void UIDraw::drawCenterOFF(String current_switch_str)
{
    Serial.println("drawCenterOff\n");
    int center = center_button_width >> 1;
    center_button->fillCircle(center, center, center-1, 1);
    center_button->drawCircle(center, center, center-2-RING_OUTSIDE_WIDTH, PALETTE_ORANGE);
    center_button->drawCircle(center, center, center-2-RING_TOTAL_WIDTH, PALETTE_ORANGE);
    for(int i = 0; i < RING_INSIDE_DIV; i++){
        center_button->fillArc(center, center, center-2-RING_OUTSIDE_WIDTH, center-2-RING_TOTAL_WIDTH, 360/RING_INSIDE_DIV*i, 360/RING_INSIDE_DIV*i, PALETTE_ORANGE);
    }
    updateButtonStr(false, current_switch_str);
}

void UIDraw::drawCenter(ButtonStatus &status, uint32_t decision_time, String current_switch_str)
{
    if(status.is_in_transition){
        if(status.is_switched_on){
            drawCenterTransitionOn2Off(status.keep_push_time, decision_time);
        }else{
            drawCenterTransitionOff2On(status.keep_push_time, decision_time);
        }
    }else{
        if(status.is_switched_on){
            drawCenterON(current_switch_str);
        }else{
            drawCenterOFF(current_switch_str);
        }
    }
    drawCenterBase();
    auto transpalette = 0;
    center_button->pushRotateZoom(0, zoom, zoom, transpalette);
    canvas->pushSprite(0, 0);
}

