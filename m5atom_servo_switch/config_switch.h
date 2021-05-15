#pragma once
#include <Arduino.h>

String SWITCH_DEF[] = {"3D PRINTER"};

// Servo
int32_t PORT_SERVO = 26;
const int32_t MIN_US = 500;
const int32_t MAX_US = 2400;
const int16_t PWR_ON_POS = 30;
const int16_t PWR_OFF_POS = 0;
const int16_t MOVE_AMOUNT = 5;
