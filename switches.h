#pragma once
#include <unordered_map>
#include <string>

enum SwitchName{
    SWITCH_HEAD = 0,
    STUDIO_LIGHT,
    PRINTER_3D,
    SWITCH_TAIL,//threshold
};

struct SwitchStatus{
    std::string str; 
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
    SwitchStatus getCurrentSwitchStatus(void);
    bool isSwitchedOnCurrentSwitch(void);
    std::string getStrCurrentSwitch(void);
    void switchOn(void);
    void switchOff(void);
    void toggleSwitch(void);
    void movedown(void);
    void moveup(void);
};