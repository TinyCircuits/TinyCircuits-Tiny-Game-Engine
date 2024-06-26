#ifndef ENGINE_IO_MODULE_H
#define ENGINE_IO_MODULE_H

#include <stdint.h>
#include "engine_io_buttons.h"

void engine_io_setup();
void engine_io_tick();

button_class_obj_t* engine_io_get_gui_toggle_button();
void engine_io_reset_gui_toggle_button();

#endif  // ENGINE_IO_MODULE_H
