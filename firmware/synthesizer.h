#pragma once
#include <stdint.h>

#define SYNTH_WAVEFORM_N 128

extern uint16_t waveform[SYNTH_WAVEFORM_N];

void synth_control(int freq_divide);
void synth_setup();
void synth_start();
void synth_stop();
void synth_interrupt();
