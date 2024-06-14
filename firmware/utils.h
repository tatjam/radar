#pragma once
#include <stdint.h>

// Absolute time in milliesconds, rough timer
extern uint32_t abs_time;

#define ABS(x) ((x<0)?(-x):x)

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
