#ifndef ENGINE_INPUT_COMMON_H
#define ENGINE_INPUT_COMMON_H

#include <stdint.h>
#include "utility/bits.h"

#define BUTTON_A            0
#define BUTTON_B            1
#define BUTTON_DPAD_UP      2
#define BUTTON_DPAD_DOWN    3
#define BUTTON_DPAD_LEFT    4
#define BUTTON_DPAD_RIGHT   5
#define BUTTON_BUMPER_LEFT  6
#define BUTTON_BUMPER_RIGHT 7

extern uint8_t engine_input_pressed_buttons;

#endif  // ENGINE_INPUT_COMMON_H