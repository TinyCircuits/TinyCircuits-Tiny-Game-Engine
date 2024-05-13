#ifndef ENGINE_IO_MODULE_H
#define ENGINE_IO_MODULE_H

#include <stdint.h>

void engine_io_setup();
void engine_io_update_pressed_buttons();

bool check_pressed(uint16_t button_mask);
bool check_just_changed(uint16_t button_mask);
bool check_just_pressed(uint16_t button_mask);
bool check_just_released(uint16_t button_mask);
uint16_t engine_io_get_gui_toggle_button();
uint16_t engine_io_reset_gui_toggle_button();

#endif  // ENGINE_IO_MODULE_H