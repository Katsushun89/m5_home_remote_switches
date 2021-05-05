#pragma once
#include <queue>

struct ButtonStatus{
    uint32_t keep_push_time;
    bool is_switched_on;
    bool is_in_transition;
};

class UIDraw
{
private:
    std::queue<ButtonStatus> event_queue;

public:
    UIDraw();
    ~UIDraw() = default;
    void pushEvent(ButtonStatus status);
    bool popEvent(ButtonStatus &status);
    //void drawCenterButton(void);
};