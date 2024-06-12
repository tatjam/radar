#pragma once
#include <stdint.h>

// Absolute time in milliesconds, rough timer
extern uint64_t abs_time;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
