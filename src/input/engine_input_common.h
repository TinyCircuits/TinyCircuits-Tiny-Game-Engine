#ifndef ENGINE_INPUT_COMMON_H
#define ENGINE_INPUT_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "utility/bits.h"

#define BUTTON_A            0b0000000000000001
#define BUTTON_B            0b0000000000000010
#define BUTTON_DPAD_UP      0b0000000000000100
#define BUTTON_DPAD_DOWN    0b0000000000001000
#define BUTTON_DPAD_LEFT    0b0000000000010000
#define BUTTON_DPAD_RIGHT   0b0000000000100000
#define BUTTON_BUMPER_LEFT  0b0000000001000000
#define BUTTON_BUMPER_RIGHT 0b0000000010000000
#define BUTTON_MENU         0b0000000100000000

extern uint16_t pressed_buttons;
extern uint16_t last_pressed_buttons;

extern uint16_t just_changed_buttons;
extern uint16_t just_pressed_buttons;
extern uint16_t just_released_buttons;

#endif  // ENGINE_INPUT_COMMON_H