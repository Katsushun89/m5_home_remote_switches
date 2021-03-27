#include "switches.h"

Switches::Switches()
{
    SwitchStatus status;
    status.is_switched_on = false;

    //initialize
    status.str = "CRAFTROOM";
    switch_status[STUDIO_LIGHT] = status;

    status.str = "3D PRINTER";
    switch_status[PRINTER_3D] = status;

    cur_switch = STUDIO_LIGHT;

}

SwitchStatus Switches::getCurrentSwitchStatus(void)
{
    return switch_status[cur_switch];
}

bool Switches::isSwitchedOnCurrentSwitch(void)
{
    return switch_status[cur_switch].is_switched_on;
}

std::string Switches::getStrCurrentSwitch(void)
{
    return switch_status[cur_switch].str;
}

void Switches::switchOn(void)
{
    SwitchStatus status = getCurrentSwitchStatus();
    status.is_switched_on = true;
    switch_status[cur_switch] = status;
}

void Switches::switchOff(void)
{
    SwitchStatus status = getCurrentSwitchStatus();
    status.is_switched_on = false;
    switch_status[cur_switch] = status;
}

void Switches::toggleSwitch(void)
{
    SwitchStatus status = getCurrentSwitchStatus();
    status.is_switched_on = !status.is_switched_on;
    switch_status[cur_switch] = status;
}

void Switches::movedown(void)
{
    cur_switch--;
    if(cur_switch <= SWITCH_HEAD){
        cur_switch = SWITCH_TAIL - 1;
    }
}

void Switches::moveup(void)
{
    cur_switch++;
    if(cur_switch >= SWITCH_TAIL){
        cur_switch = SWITCH_HEAD + 1;
    }
}