#pragma once
#include <stdbool.h>
#include <stdint.h>

#define SYNTH_WAVEFORM_N 512

extern uint16_t waveform[SYNTH_WAVEFORM_N];

void synth_control(int freq_divide);
void synth_setup();
void synth_start();
void synth_stop();
void synth_interrupt();

void synth_chirp_waveform(uint16_t f0, uint16_t fend, uint16_t num_steps);
void synth_stepped_waveform(uint16_t* steps, uint16_t num_steps);
void synth_pcm_waveform(uint16_t* steps, uint16_t num_steps);

void vco_set_running(bool value);
