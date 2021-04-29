#pragma once
#include <unordered_map>
//#include <string>
#include <Arduino.h>

enum SwitchName{
    SWITCH_HEAD = 0,
    STUDIO_LIGHT,
    PRINTER_3D,
    SWITCH_TAIL,//threshold
};

struct SwitchStatus{
    String str;
    String firebase_path;
    bool is_switched_on;
};

class Switches
{
private:
    std::unordered_map<uint32_t, SwitchStatus> switch_status;
    uint32_t cur_switch;

public:
    Switches();
    ~Switches() = default;
    void setup(void);
    SwitchStatus getCurrentSwitchStatus(void);
    bool isSwitchedOnCurrentSwitch(void);
    String getStrCurrentSwitch(void);
    String getFirebasePathCurrentSwitch(void);
    String getSwitchName(uint32_t switch_number);
    void setFirebasePath(uint32_t switch_number, String str);
    String getFirebasePath(uint32_t switch_number);
    bool toggleSwitch(void);
    void movedown(void);
    void moveup(void);
    void updatePowerStatus(String path, bool power);
};