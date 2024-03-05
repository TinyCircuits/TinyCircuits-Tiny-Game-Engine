#include "engine_input_common.h"

uint16_t pressed_buttons = 0b0000000000000000;
uint16_t last_pressed_buttons = 0b0000000000000000;

uint16_t just_changed_buttons = 0b0000000000000000;
uint16_t just_pressed_buttons = 0b0000000000000000;
uint16_t just_released_buttons = 0b0000000000000000;