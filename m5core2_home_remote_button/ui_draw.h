#pragma once
#include <LovyanGFX.hpp>
#include <queue>

struct ButtonStatus {
    uint32_t keep_push_time;
    bool is_switched_on;
    bool is_in_transition;
};

class UIDraw {
   private:
    std::queue<ButtonStatus> event_queue;
    LGFX *lcd;
    LGFX_Sprite *canvas;         //(&lcd);
    LGFX_Sprite *center_base;    //(&canvas);
    LGFX_Sprite *center_button;  //(&canvas);

    int center_px;
    int center_py;
    int32_t lcd_width;

    float zoom;
    int center_button_width;

    static constexpr int PALETTE_BLACK = 1;
    static constexpr int PALETTE_ORANGE = 2;
    static constexpr uint32_t RING_INSIDE_WIDTH = 20;
    static constexpr uint32_t RING_OUTSIDE_WIDTH = 4;
    static constexpr uint32_t RING_TOTAL_WIDTH =
        RING_OUTSIDE_WIDTH + RING_INSIDE_WIDTH;
    static constexpr uint8_t RING_INSIDE_DIV = 6;

    void drawLRButton(void);
    void drawCenterBase(void);
    void drawCenterTransitionOff2On(uint32_t keep_push_time,
                                    uint32_t decision_time);
    void drawCenterTransitionOn2Off(uint32_t keep_push_time,
                                    uint32_t decision_time);
    void updateButtonStr(bool onOff, String current_switch_str);
    void drawCenterON(String current_switch_str);
    void drawCenterOFF(String current_switch_str);

   public:
    UIDraw();
    ~UIDraw() = default;
    void setup(void);
    void pushEvent(ButtonStatus status);
    bool popEvent(ButtonStatus &status);
    // void drawCenterButton(void);
    int getLcdWidth(void) { return lcd_width; };
    int getCenterPx(void) { return center_px; };
    int getCenterPy(void) { return center_py; };
    void drawCenter(ButtonStatus &status, uint32_t decision_time,
                    String current_switch_str);
    static constexpr int32_t CENTER_BUTTON_WIDTH = 240;
};