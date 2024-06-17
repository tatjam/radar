#pragma once
#include <stdint.h>
#include <stdbool.h>


// We double-buffer, trying to send to USB half the buffer
// while the other half is being written. So we go for the
// 1024 byte limit (note this is 2 bytes per sample!)
// TODO: We could try smartly "compressing" the data or using
// 8 bit samples if this proves too slow
#define ADC_DATABUFFER 1024
extern uint8_t adc_buffer[ADC_DATABUFFER];

// 0 = no data to process
// 1 = first half ready
// 2 = second half ready
// Clear after reading
extern int buffer_status;

void adc_setup();
void adc_calibrate();
void adc_start();
void adc_stop();
void adc_interrupt(bool half);
