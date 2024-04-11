#ifndef ENGINE_INPUT_MODULE_H
#define ENGINE_INPUT_MODULE_H

void engine_input_setup();
void engine_input_update_pressed_buttons();

bool check_pressed(uint16_t button_mask);
bool check_just_changed(uint16_t button_mask);
bool check_just_pressed(uint16_t button_mask);
bool check_just_released(uint16_t button_mask);
uint16_t engine_input_get_gui_toggle_button();

#endif  // ENGINE_INPUT_MODULE_H