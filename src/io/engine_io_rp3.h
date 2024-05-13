#ifndef ENGINE_IO_RP3_H
#define ENGINE_IO_RP3_H

#include <stdbool.h>

void engine_io_rp3_setup();
void engine_io_rp3_update_pressed_mask();
void engine_io_rp3_rumble(float intensity);

#endif  // ENGINE_INPUT_RP3_H