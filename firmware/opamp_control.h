#pragma once
#include <stdint.h>

#define OPAMP_PREAMP 0
#define OPAMP_AMP 1
#define OPAMP_BOTH 2
// Shared U/D line, CS is not shared

// Positive steps increase, negative decrease
void opamp_control(int steps, int select);


void opamp_setup();