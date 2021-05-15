#include "switches.h"

Switches::Switches(String switch_names[], size_t switch_num) {
    SwitchStatus status;
    status.is_switched_on = false;

    for (int i = 0; i < switch_num; i++) {
        status.str = switch_names[i];
        switch_status[i] = status;
        setFirebasePath(i, "/" + switch_names[i]);
    }

    cur_switch = 0;
}

void Switches::updatePowerStatus(String path, bool power) {
    for (auto status : switch_status) {
        if (path.startsWith(status.second.firebase_path)) {
            status.second.is_switched_on = power;
        }
    }
}

void Switches::updatePowerStatus(int32_t switch_number, bool power) {
    switch_status[switch_number].is_switched_on = power;
}

int32_t Switches::getCurrentSwitchNumber(void) { return cur_switch; }

SwitchStatus Switches::getCurrentSwitchStatus(void) {
    return switch_status[cur_switch];
}

bool Switches::isSwitchedOnCurrentSwitch(void) {
    return switch_status[cur_switch].is_switched_on;
}

String Switches::getStrCurrentSwitch(void) {
    return switch_status[cur_switch].str;
}

String Switches::getFirebasePathCurrentSwitch(void) {
    return switch_status[cur_switch].firebase_path;
}

String Switches::getSwitchName(int32_t switch_number) {
    return switch_status[switch_number].str;
}

void Switches::setFirebasePath(int32_t switch_number, String str) {
    switch_status[switch_number].firebase_path = str;
}

String Switches::getFirebasePath(int32_t switch_number) {
    return switch_status[switch_number].firebase_path;
}

bool Switches::toggleSwitch(void) {
    SwitchStatus status = getCurrentSwitchStatus();
    status.is_switched_on = !status.is_switched_on;

    switch_status[cur_switch] = status;
    return status.is_switched_on;
}

void Switches::movedown(void) {
    cur_switch--;
    if (cur_switch < 0) {
        cur_switch = switch_status.size() - 1;
    }
}

void Switches::moveup(void) {
    cur_switch++;
    if (cur_switch >= switch_status.size()) {
        cur_switch = 0;
    }
}