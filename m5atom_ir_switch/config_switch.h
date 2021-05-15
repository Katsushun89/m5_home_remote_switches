#pragma once
#include <Arduino.h>

String SWITCH_DEF[] = {"CRAFTROOM"};

// IR
const uint16_t PORT_IR = 26;

const uint64_t ir_remote_cmd[] = {
    0xE730D12EUL,  // OFF
    0xE730E916UL,  // ON
};
