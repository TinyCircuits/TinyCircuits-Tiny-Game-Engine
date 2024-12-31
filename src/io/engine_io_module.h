#ifndef ENGINE_IO_MODULE_H
#define ENGINE_IO_MODULE_H

#include <stdint.h>
#include "engine_io_buttons.h"

float engine_io_raw_battery_level();
void engine_io_setup();
void engine_io_reset();
void engine_io_battery_monitor_setup();
void engine_io_tick();

#endif  // ENGINE_IO_MODULE_H
