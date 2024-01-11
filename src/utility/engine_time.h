#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

#include <stdint.h>

// Returns milliseconds since some time (IDK)
uint32_t millis();
void cycles_start();
uint32_t cycles_stop();

#endif  // ENGINE_TIME_H