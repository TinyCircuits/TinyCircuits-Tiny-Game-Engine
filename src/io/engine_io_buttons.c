#include "debug/debug_print.h"
#include "engine_gui.h"
#include "engine_io_buttons.h"
#include "py/obj.h"

#ifdef __unix__
    #include "engine_io_sdl.h"
#else
    #include "engine_io_rp3.h"
    #include "hardware/adc.h"
#endif


#define BUTTON_DEFAULT_LONG_PRESS_TIME          400
#define BUTTON_DEFAULT_DOUBLE_PRESS_TIME        300
#define BUTTON_DEFAULT_AUTOREPEAT_DELAY_TIME    400
#define BUTTON_DEFAULT_AUTOREPEAT_INTERVAL_TIME 100

#define NEW_BUTTON(btn_name, btn_code) { \
    {&button_class_type}, \
    .name = btn_name, \
    .code = btn_code, \
    .long_press_time = BUTTON_DEFAULT_LONG_PRESS_TIME, \
    .double_press_time = BUTTON_DEFAULT_DOUBLE_PRESS_TIME, \
    .autorepeat_delay_time = BUTTON_DEFAULT_AUTOREPEAT_DELAY_TIME, \
    .autorepeat_interval_time = BUTTON_DEFAULT_AUTOREPEAT_INTERVAL_TIME, \
    .last_pressed_millis = MILLIS_NULL, \
    .last_released_millis = MILLIS_NULL, \
}

button_class_obj_t BUTTON_DPAD_UP = NEW_BUTTON("UP", BUTTON_CODE_DPAD_UP);
button_class_obj_t BUTTON_DPAD_DOWN = NEW_BUTTON("DOWN", BUTTON_CODE_DPAD_DOWN);
button_class_obj_t BUTTON_DPAD_LEFT = NEW_BUTTON("LEFT", BUTTON_CODE_DPAD_LEFT);
button_class_obj_t BUTTON_DPAD_RIGHT = NEW_BUTTON("RIGHT", BUTTON_CODE_DPAD_RIGHT);
button_class_obj_t BUTTON_A = NEW_BUTTON("A", BUTTON_CODE_A);
button_class_obj_t BUTTON_B = NEW_BUTTON("B", BUTTON_CODE_B);
button_class_obj_t BUTTON_BUMPER_LEFT = NEW_BUTTON("LB", BUTTON_CODE_BUMPER_LEFT);
button_class_obj_t BUTTON_BUMPER_RIGHT = NEW_BUTTON("RB", BUTTON_CODE_BUMPER_RIGHT);
button_class_obj_t BUTTON_MENU = NEW_BUTTON("MENU", BUTTON_CODE_MENU);


uint16_t pressed_buttons = 0;
uint16_t long_pressed_buttons = 0;
uint16_t double_pressed_buttons = 0;
uint16_t autorepeat_buttons = 0;

uint16_t prev_pressed_buttons = 0;
uint16_t prev_long_pressed_buttons = 0;

uint16_t assume_released_buttons = ~0;


uint32_t prev_tick_millis = MILLIS_NULL;
// The reference time from which autorepeat is calculated. It is set when autorepeat occurs, and cleared when buttons are released.
uint32_t autorepeat_base_millis = MILLIS_NULL;

void buttons_update_state() {
    // Clear the double-pressed state for released buttons.
    // Note that this is done at the beginning of the update function, so that clearing
    // the double-pressed state is delayed by one tick. This helps with determining if a release
    // was a double press or not.
    double_pressed_buttons &= pressed_buttons;

    // Move the current state to prev_*.
    prev_pressed_buttons = pressed_buttons;
    prev_long_pressed_buttons = long_pressed_buttons;
    // Store the current state of the buttons in pressed_buttons.
    #ifdef __unix__
        pressed_buttons = engine_io_sdl_pressed_buttons();
    #else
        pressed_buttons = engine_io_rp3_pressed_buttons();
    #endif
    // Clear the assume-released state for any actually released button.
    assume_released_buttons &= pressed_buttons;
    // Mark all the assume-released buttons as not pressed.
    pressed_buttons &= ~assume_released_buttons;
    // Clear the long-pressed state for released buttons.
    long_pressed_buttons &= pressed_buttons;
    // Clear the autorepeat state, individual buttons might set it for one tick only.
    autorepeat_buttons = 0;
    // Clear the autorepeat base if all buttons are released.
    if (!pressed_buttons) {
        autorepeat_base_millis = MILLIS_NULL;
    }

    uint32_t now_millis = millis();
    int32_t tick_time = prev_tick_millis == MILLIS_NULL ? 0 : millis_diff(now_millis, prev_tick_millis);
    // Call update on each button. This sets the long press, double press and autorepeat states.
    // These operations cannot be done globally as they rely on constants which can be configured per button.
    button_update_state(&BUTTON_DPAD_UP, now_millis, tick_time);
    button_update_state(&BUTTON_DPAD_DOWN, now_millis, tick_time);
    button_update_state(&BUTTON_DPAD_LEFT, now_millis, tick_time);
    button_update_state(&BUTTON_DPAD_RIGHT, now_millis, tick_time);
    button_update_state(&BUTTON_A, now_millis, tick_time);
    button_update_state(&BUTTON_B, now_millis, tick_time);
    button_update_state(&BUTTON_BUMPER_LEFT, now_millis, tick_time);
    button_update_state(&BUTTON_BUMPER_RIGHT, now_millis, tick_time);
    button_update_state(&BUTTON_MENU, now_millis, tick_time);
    prev_tick_millis = now_millis;
}

void button_update_state(button_class_obj_t *button, uint32_t now_millis, int32_t tick_time) {
    uint16_t code = button->code;
    if (code & pressed_buttons) {
        if (code & prev_pressed_buttons) {  // Still pressed
            int32_t pressed_time = millis_diff(now_millis, button->last_pressed_millis);
            // Check for a long press.
            if (pressed_time >= button->long_press_time) {
                long_pressed_buttons |= code;
            }
            // Check for autorepeat.
            if (pressed_time >= button->autorepeat_delay_time) {
                // Set the base for autorepeat. Another autorepeated button will use the same base, so if it has
                // the same interval, autorepeat ticks will be synchronised between them.
                if (autorepeat_base_millis == MILLIS_NULL) {
                    autorepeat_base_millis = now_millis;
                }
                if (millis_diff(now_millis, autorepeat_base_millis) % button->autorepeat_interval_time < tick_time) {
                    autorepeat_buttons |= code;
                }
            }
        } else { // Just pressed
            autorepeat_buttons |= code;
            // Check for double press. It happens if the released time was less than the double press time, and the
            // previous press was not long.
            if (!(code & double_pressed_buttons) &&
                    button->last_released_millis != MILLIS_NULL &&
                    button->last_pressed_millis != MILLIS_NULL) {
                int32_t released_time = millis_diff(now_millis, button->last_released_millis);
                if (released_time <= button->double_press_time) {
                    int32_t previous_pressed_time = millis_diff(button->last_released_millis, button->last_pressed_millis);
                    if (previous_pressed_time <= button->long_press_time) {
                        double_pressed_buttons |= code;
                    }
                }
            }
            button->last_pressed_millis = now_millis;
        }
    } else if (code & prev_pressed_buttons) {  // Just released
        button->last_released_millis = now_millis;
    }
}


inline void button_release_helper(button_class_obj_t *button) {
    button->last_pressed_millis = MILLIS_NULL;
    button->last_released_millis = MILLIS_NULL;
}

void buttons_release_all() {
    pressed_buttons = 0;
    long_pressed_buttons = 0;
    double_pressed_buttons = 0;
    autorepeat_buttons = 0;
    prev_pressed_buttons = 0;
    prev_long_pressed_buttons = 0;
    assume_released_buttons = ~0;
    button_release_helper(&BUTTON_DPAD_UP);
    button_release_helper(&BUTTON_DPAD_DOWN);
    button_release_helper(&BUTTON_DPAD_LEFT);
    button_release_helper(&BUTTON_DPAD_RIGHT);
    button_release_helper(&BUTTON_A);
    button_release_helper(&BUTTON_B);
    button_release_helper(&BUTTON_BUMPER_LEFT);
    button_release_helper(&BUTTON_BUMPER_RIGHT);
    button_release_helper(&BUTTON_MENU);
}

void button_release(button_class_obj_t *button) {
    uint16_t code = button->code;
    pressed_buttons &= ~code;
    long_pressed_buttons &= ~code;
    double_pressed_buttons &= ~code;
    autorepeat_buttons &= ~code;
    prev_pressed_buttons &= ~code;
    prev_long_pressed_buttons &= ~code;
    assume_released_buttons |= code;
    button_release_helper(button);
}


void buttons_reset_params_all() {
    button_reset_params(&BUTTON_DPAD_UP);
    button_reset_params(&BUTTON_DPAD_DOWN);
    button_reset_params(&BUTTON_DPAD_LEFT);
    button_reset_params(&BUTTON_DPAD_RIGHT);
    button_reset_params(&BUTTON_A);
    button_reset_params(&BUTTON_B);
    button_reset_params(&BUTTON_BUMPER_LEFT);
    button_reset_params(&BUTTON_BUMPER_RIGHT);
    button_reset_params(&BUTTON_MENU);
}

void button_reset_params(button_class_obj_t *button) {
    button->long_press_time = BUTTON_DEFAULT_LONG_PRESS_TIME;
    button->double_press_time = BUTTON_DEFAULT_DOUBLE_PRESS_TIME;
    button->autorepeat_delay_time = BUTTON_DEFAULT_AUTOREPEAT_DELAY_TIME;
    button->autorepeat_interval_time = BUTTON_DEFAULT_AUTOREPEAT_INTERVAL_TIME;
}


int32_t button_press_time(button_class_obj_t *button) {
    if (!button_is_pressed(button) || button->last_pressed_millis == MILLIS_NULL) {
        return 0;
    }
    return millis_diff(millis(), button->last_pressed_millis);
}

inline bool button_is_pressed(button_class_obj_t *button) {
    return (button->code & pressed_buttons) != 0;
}

inline bool button_is_just_pressed(button_class_obj_t *button) {
    return (button->code & (pressed_buttons & ~prev_pressed_buttons)) != 0;
}

inline bool button_is_just_released(button_class_obj_t *button) {
    return (button->code & (~pressed_buttons & prev_pressed_buttons)) != 0;
}

inline bool button_is_long_pressed(button_class_obj_t *button) {
    return (button->code & long_pressed_buttons) != 0;
}

inline bool button_is_just_long_pressed(button_class_obj_t *button) {
    return (button->code & (long_pressed_buttons & ~prev_long_pressed_buttons)) != 0;
}

inline bool button_is_just_short_released(button_class_obj_t *button) {
    return (button->code & (~pressed_buttons & prev_pressed_buttons & ~prev_long_pressed_buttons)) != 0;
}

inline bool button_is_just_long_released(button_class_obj_t *button) {
    return (button->code & (~pressed_buttons & prev_pressed_buttons & prev_long_pressed_buttons)) != 0;
}

inline bool button_is_just_double_pressed(button_class_obj_t *button) {
    return (button->code & (double_pressed_buttons & ~prev_pressed_buttons)) != 0;
}

inline bool button_is_double_pressed(button_class_obj_t *button) {
    return (button->code & double_pressed_buttons) != 0;
}

inline bool button_is_just_double_released(button_class_obj_t *button) {
    return (button->code & (~pressed_buttons & prev_pressed_buttons & double_pressed_buttons)) != 0;
}

inline bool button_is_pressed_autorepeat(button_class_obj_t *button) {
    return (button->code & autorepeat_buttons);
}


static void button_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination) {
    button_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (destination[0] == MP_OBJ_NULL) { // Load
        switch (attribute) {
            case MP_QSTR_name:
                destination[0] = mp_obj_new_str(self->name, strlen(self->name));
            break;
            // Parameters:
            case MP_QSTR_long_press_time:
                destination[0] = mp_obj_new_int(self->long_press_time);
            break;
            case MP_QSTR_autorepeat_delay_time:
                destination[0] = mp_obj_new_int(self->autorepeat_delay_time);
            break;
            case MP_QSTR_autorepeat_interval_time:
                destination[0] = mp_obj_new_int(self->autorepeat_interval_time);
            break;
            case MP_QSTR_double_press_time:
                destination[0] = mp_obj_new_int(self->double_press_time);
            break;
            // Press state:
            // Make all the buttons act as if released and release if the GUI is focused.
            case MP_QSTR_press_time:
                destination[0] = mp_obj_new_int(engine_gui_is_gui_focused() ? 0 : button_press_time(self));
            break;
            case MP_QSTR_is_pressed:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_pressed(self));
            break;
            case MP_QSTR_is_just_pressed:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_pressed(self));
            break;
            case MP_QSTR_is_just_released:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_released(self));
            break;
            case MP_QSTR_is_long_pressed:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_long_pressed(self));
            break;
            case MP_QSTR_is_just_long_pressed:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_long_pressed(self));
            break;
            case MP_QSTR_is_just_short_released:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_short_released(self));
            break;
            case MP_QSTR_is_just_long_released:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_long_released(self));
            break;
            case MP_QSTR_is_just_double_pressed:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_double_pressed(self));
            break;
            case MP_QSTR_is_double_pressed:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_double_pressed(self));
            break;
            case MP_QSTR_is_just_double_released:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_just_double_released(self));
            break;
            case MP_QSTR_is_pressed_autorepeat:
                destination[0] = mp_obj_new_bool(!engine_gui_is_gui_focused() && button_is_pressed_autorepeat(self));
            break;
            default:
                destination[1] = MP_OBJ_SENTINEL; // Continue in locals_dict.
        }
    } else if (destination[1] != MP_OBJ_NULL) { // Store
        // Parameters:
        switch (attribute) {
            case MP_QSTR_long_press_time:
                self->long_press_time = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_autorepeat_delay_time:
                self->autorepeat_delay_time = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_autorepeat_interval_time:
                self->autorepeat_interval_time = mp_obj_get_int(destination[1]);
            break;
            case MP_QSTR_double_press_time:
                self->double_press_time = mp_obj_get_int(destination[1]);
            break;
            default:
                return; // Fail
        }
        destination[0] = MP_OBJ_NULL; // Success
    }
}


/*  --- doc ---
    NAME: release
    ID: release
    DESC: Treat the button as if released, until the next press.
    RETURN: None
*/
static mp_obj_t button_release_method(mp_obj_t self_in) {
    button_release(MP_OBJ_TO_PTR(self_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(button_release_obj, button_release_method);


/*  --- doc ---
    NAME: reset_params
    ID: reset_params
    DESC: Reset the button parameters to their default values.
    RETURN: None
*/
static mp_obj_t button_reset_params_method(mp_obj_t self_in) {
    button_reset_params(MP_OBJ_TO_PTR(self_in));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(button_reset_params_obj, button_reset_params_method);


#define INFO_TRUE_MARK          " + "
#define INFO_JUST_TRUE_MARK     "[+]"
#define INFO_FALSE_MARK         "   "
#define INFO_JUST_FALSE_MARK    "[ ]"

/*  --- doc ---
    NAME: print_info
    ID: print_info
    DESC: Print the state of the button to the console.
    PARAM: [type=bool (optional)]   [name=print_header]  [value=bool, default: True]
    RETURN: None

*/
static mp_obj_t button_print_info(size_t n_args, const mp_obj_t *args) {
    button_class_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    bool print_header = n_args == 1 || mp_obj_is_true(args[1]);
    if (print_header) {
        mp_printf(&mp_plat_print, "                      press  long   short  long    dbl    dbl   auto\n");
        mp_printf(&mp_plat_print, "Button  press   rel   time   press   rel    rel   press   rel    rep\n");
    }
    mp_printf(&mp_plat_print, "%-6s   %s    %s   %5d   %s    %s    %s    %s    %s    %s\n",
        self->name,
        button_is_just_pressed(self) ? INFO_JUST_TRUE_MARK :
            button_is_pressed(self) ? INFO_TRUE_MARK : INFO_FALSE_MARK,
        button_is_just_released(self) ? INFO_JUST_TRUE_MARK :
            button_is_pressed(self) ? INFO_FALSE_MARK : INFO_TRUE_MARK,
        button_press_time(self),
        button_is_just_long_pressed(self) ? INFO_JUST_TRUE_MARK :
            button_is_long_pressed(self) ? INFO_TRUE_MARK : INFO_FALSE_MARK,
        button_is_just_short_released(self) ? INFO_JUST_TRUE_MARK : INFO_FALSE_MARK,
        button_is_just_long_released(self) ? INFO_JUST_TRUE_MARK : INFO_FALSE_MARK,
        button_is_just_double_pressed(self) ? INFO_JUST_TRUE_MARK :
            button_is_double_pressed(self) ? INFO_TRUE_MARK : INFO_FALSE_MARK,
        button_is_just_double_released(self) ? INFO_JUST_TRUE_MARK : INFO_FALSE_MARK,
        button_is_pressed_autorepeat(self) ? INFO_TRUE_MARK : INFO_FALSE_MARK
    );
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(button_print_info_obj, 1, 2, button_print_info);


const mp_rom_map_elem_t button_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_release), MP_ROM_PTR(&button_release_obj) },
    { MP_ROM_QSTR(MP_QSTR_reset_params), MP_ROM_PTR(&button_reset_params_obj) },
    { MP_ROM_QSTR(MP_QSTR_print_info), MP_ROM_PTR(&button_print_info_obj) },
};
MP_DEFINE_CONST_DICT(button_class_locals_dict, button_class_locals_dict_table);


static void button_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    button_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ENGINE_PRINTF("Button:%s", self->name);
}


/*  --- doc ---
    NAME: Button
    ID: Button
    DESC: Button class allowing to check the state of a button. Note: If the GUI is focused, all buttons are treated as released.
    ATTR:   [type=str]              [name=name]                         [value=string]

    ATTR:   [type=int]              [name=long_press_time]              [value=time in ms for a press to be considered long]
    ATTR:   [type=int]              [name=double_press_time]            [value=time between two presses to treat them as a double press]
    ATTR:   [type=int]              [name=autorepeat_delay_time]        [value=time before autorepeat starts]
    ATTR:   [type=int]              [name=autorepeat_interval_time]     [value=time between autorepeats]

    ATTR:   [type=int]              [name=press_time]                   [value=if pressed, time since the button was last pressed, otherwise 0]
    ATTR:   [type=bool]             [name=is_pressed]                   [value=whether currently pressed]
    ATTR:   [type=bool]             [name=is_just_pressed]              [value=whether became pressed this tick]
    ATTR:   [type=bool]             [name=is_just_released]             [value=whether became released (not pressed) this tick]
    ATTR:   [type=bool]             [name=is_long_pressed]              [value=whether currently being long pressed]
    ATTR:   [type=bool]             [name=is_just_long_pressed]         [value=whether currently being pressed, and just became long pressed this tick - RECOMMENDED FOR DETECTING A LONG PRESS]
    ATTR:   [type=bool]             [name=is_just_short_released]       [value=whether just released after a short press - RECOMMENDED FOR DETECTING A SHORT PRESS]
    ATTR:   [type=bool]             [name=is_just_long_released]        [value=whether just released after a long press]
    ATTR:   [type=bool]             [name=is_just_double_pressed]       [value=whether just became pressed shortly after a previous short press]
    ATTR:   [type=bool]             [name=is_double_pressed]            [value=whether currently pressed, shortly after a previous short press (the second press can be long)]
    ATTR:   [type=bool]             [name=is_just_double_released]      [value=whether just released after a double press - RECOMMENDED FOR DETECTING A DOUBLE PRESS]
    ATTR:   [type=bool]             [name=is_pressed_autorepeat]        [value=whether this tick should invoke an autorepeated action - RECOMMENDED FOR AUTOREPEATED ACTIONS LIKE SCROLLING]

    ATTR:   [type=function]         [name={ref_link:release}]           [value=function]
    ATTR:   [type=function]         [name={ref_link:reset_params}]      [value=function]
    ATTR:   [type=function]         [name={ref_link:print_info}]        [value=function]
*/
MP_DEFINE_CONST_OBJ_TYPE(
    button_class_type,
    MP_QSTR_Button,
    MP_TYPE_FLAG_NONE,

    attr, button_class_attr,
    locals_dict, (mp_obj_dict_t*)&button_class_locals_dict,
    print, button_class_print
);
