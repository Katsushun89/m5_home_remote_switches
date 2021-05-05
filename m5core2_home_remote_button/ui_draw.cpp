#include <LovyanGFX.hpp>
#include "ui_draw.h"

UIDraw::UIDraw()
{
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

