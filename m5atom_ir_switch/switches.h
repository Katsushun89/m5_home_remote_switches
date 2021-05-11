#pragma once

#include <unordered_map>
#include <Arduino.h>

struct SwitchStatus{
    String str;
    String firebase_path;
    bool is_switched_on;
};

class Switches
{
private:
    std::unordered_map<int32_t, SwitchStatus> switch_status;
    int32_t cur_switch;

public:
    Switches(String switch_names[], size_t switch_num);
    ~Switches() = default;
    void setup(void);
    int32_t getCurrentSwitchNumber(void);
    SwitchStatus getCurrentSwitchStatus(void);
    bool isSwitchedOnCurrentSwitch(void);
    String getStrCurrentSwitch(void);
    String getFirebasePathCurrentSwitch(void);
    String getSwitchName(int32_t switch_number);
    void setFirebasePath(int32_t switch_number, String str);
    String getFirebasePath(int32_t switch_number);
    bool toggleSwitch(void);
    void movedown(void);
    void moveup(void);
    void updatePowerStatus(String path, bool power);
    void updatePowerStatus(int32_t switch_number, bool power);

};