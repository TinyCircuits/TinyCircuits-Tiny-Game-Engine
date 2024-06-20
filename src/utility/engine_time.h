#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

#include <stdint.h>

// Period of values returned by millis(). Should be a power of two.
#define MILLIS_PERIOD 0x10000000

// Returns milliseconds since some time (IDK)
uint32_t millis();
int32_t millis_diff(uint32_t end, uint32_t start);
uint32_t millis_add(uint32_t millis, int32_t delta);

void cycles_start();
uint32_t cycles_stop();

#endif  // ENGINE_TIME_H
