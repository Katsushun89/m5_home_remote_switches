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

void Switches::updatePowerStatus(String path, bool power)
{
    for(int i = SWITCH_HEAD + 1; i < SWITCH_TAIL; i++){
        if(path.startsWith(switch_status[i].firebase_path)){
            switch_status[i].is_switched_on = power;
            Serial.printf("fb_path:%s update power %d\n", switch_status[i].firebase_path.c_str(), switch_status[i].is_switched_on);
        }
    }
}

SwitchStatus Switches::getCurrentSwitchStatus(void)
{
    return switch_status[cur_switch];
}

bool Switches::isSwitchedOnCurrentSwitch(void)
{
    return switch_status[cur_switch].is_switched_on;
}

String Switches::getStrCurrentSwitch(void)
{
    return switch_status[cur_switch].str;
}

String Switches::getFirebasePathCurrentSwitch(void)
{
    return switch_status[cur_switch].firebase_path;
}

String Switches::getSwitchName(uint32_t switch_number)
{
    return switch_status[switch_number].str;
}


void Switches::setFirebasePath(uint32_t switch_number, String str)
{
    switch_status[switch_number].firebase_path = str;
}

String Switches::getFirebasePath(uint32_t switch_number)
{
    return switch_status[switch_number].firebase_path;
}

bool Switches::toggleSwitch(void)
{
    SwitchStatus status = getCurrentSwitchStatus();
    status.is_switched_on = !status.is_switched_on;

    switch_status[cur_switch] = status;
    return status.is_switched_on;
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