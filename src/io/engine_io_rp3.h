#ifndef ENGINE_IO_RP3_H
#define ENGINE_IO_RP3_H

#include <stdbool.h>
#include <stdint.h>

void engine_io_rp3_set_indicator_color(uint16_t color);
void engine_io_rp3_set_indicator_state(bool on);

void engine_io_rp3_setup();
uint16_t engine_io_rp3_pressed_buttons();
void engine_io_rp3_rumble(float intensity);
bool engine_io_rp3_is_charging();

#endif // ENGINE_INPUT_RP3_H
