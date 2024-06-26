#ifndef ENGINE_IO_BUTTONS_H
#define ENGINE_IO_BUTTONS_H

#include <stdint.h>
#include "py/obj.h"
#include "engine_io_button_codes.h"


extern uint16_t pressed_buttons;
extern uint16_t long_pressed_buttons;
extern uint16_t double_pressed_buttons;

extern uint16_t prev_pressed_buttons;
extern uint16_t prev_long_pressed_buttons;

extern uint16_t assume_released_buttons;


typedef struct {
    mp_obj_base_t base;
    char *name;
    uint16_t code;

    // Params:

    int32_t long_press_time;
    int32_t double_press_time;
    int32_t autorepeat_delay_time;
    int32_t autorepeat_interval_time;

    // State:

    // The value of millis() at the time the button last became pressed.
    uint32_t last_pressed_millis;
    // The value of millis() at the time the button last became released.
    uint32_t last_released_millis;
} button_class_obj_t;

extern const mp_obj_type_t button_class_type;


extern button_class_obj_t BUTTON_DPAD_UP;
extern button_class_obj_t BUTTON_DPAD_DOWN;
extern button_class_obj_t BUTTON_DPAD_LEFT;
extern button_class_obj_t BUTTON_DPAD_RIGHT;
extern button_class_obj_t BUTTON_A;
extern button_class_obj_t BUTTON_B;
extern button_class_obj_t BUTTON_BUMPER_LEFT;
extern button_class_obj_t BUTTON_BUMPER_RIGHT;
extern button_class_obj_t BUTTON_MENU;


void buttons_update_state();
void button_update_state(button_class_obj_t *button, uint32_t now_millis);


void buttons_release_all();
void button_release(button_class_obj_t *button);

void buttons_reset_params_all();
void button_reset_params(button_class_obj_t *button);

bool button_is_pressed(button_class_obj_t *button);
bool button_is_just_pressed(button_class_obj_t *button);
bool button_is_just_released(button_class_obj_t *button);

bool button_is_long_pressed(button_class_obj_t *button);
bool button_is_just_long_pressed(button_class_obj_t *button);
bool button_is_just_short_released(button_class_obj_t *button);
bool button_is_just_long_released(button_class_obj_t *button);

bool button_is_just_double_pressed(button_class_obj_t *button);
bool button_is_double_pressed(button_class_obj_t *button);
bool button_is_just_double_released(button_class_obj_t *button);

bool button_is_pressed_autorepeat(button_class_obj_t *button);

#endif // ENGINE_IO_BUTTONS_H
