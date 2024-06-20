#include "py/obj.h"
#include "engine_io_common.h"
#include "engine_gui.h"
#include "engine_main.h"
#include "math/engine_math.h"
#include "utility/engine_defines.h"


#ifdef __unix__
    #include "engine_io_sdl.h"
#else
    #include "engine_io_rp3.h"
    #include "hardware/adc.h"
#endif


uint16_t gui_toggle_button = BUTTON_MENU;


void engine_io_setup(){
    #ifdef __unix__
        // Nothing to do
    #else
        engine_io_rp3_setup();
    #endif
}


// Update 'pressed_buttons' (usually called once per game loop)
void engine_io_update_pressed_buttons(){
    #ifdef __unix__
        engine_io_sdl_update_pressed_mask();
    #else
        engine_io_rp3_update_pressed_mask();
    #endif

    // `pressed_buttons` always has bits set to 1 when pressed
    // and 0 when not after the above functions are called

    // XOR: set `1` if the corresponding bits in the byte are different
    just_changed_buttons = last_pressed_buttons ^ pressed_buttons;

    // Figure out which buttons just changed from 0 (not pressed)
    // to 1 (pressed) since last time. Steps:
    // 1. Take the NOR of the `last_pressed_buttons` with all
    //    `0x0000` to get bits set to 1 of NOT pressed buttons
    // 2. Take the AND against the above for now `pressed_buttons`
    just_pressed_buttons = (~(last_pressed_buttons | 0x0000)) & pressed_buttons;

    // Figure out which buttons just changed from 1 (pressed)
    // to 0 (not pressed) since last time.
    just_released_buttons = (~(pressed_buttons | 0x0000)) & last_pressed_buttons;

    // Keep track of the buttons states from last time
    last_pressed_buttons = pressed_buttons;
}


bool check_pressed(uint16_t button_mask){
    // Check that the bits in the input button mask and the bits
    // in the internal button mask are all exactly on.
    return (pressed_buttons & button_mask) == button_mask;
}


bool check_just_changed(uint16_t button_mask){
    return (just_changed_buttons & button_mask) == button_mask;
}


bool check_just_pressed(uint16_t button_mask){
    return (just_pressed_buttons & button_mask) == button_mask;
}


bool check_just_released(uint16_t button_mask){
    return (just_released_buttons & button_mask) == button_mask;
}


uint16_t engine_io_get_gui_toggle_button(){
    return gui_toggle_button;
}


void engine_io_reset_gui_toggle_button(){
    gui_toggle_button = BUTTON_MENU;
}


/*  --- doc ---
    NAME: check_pressed
    ID: check_pressed
    DESC: For checking button presses. OR'ing together values means this returns true when all OR'ed buttons are pressed
    PARAM: [type=int]   [name=button_mask]  [value=single or OR'ed together enum/ints (e.g. 'engine_input.A | engine_input.B')]
    RETURN: True or False
*/
static mp_obj_t engine_io_check_pressed(mp_obj_t button_mask_u16){
    if(engine_gui_is_gui_focused() == true){
        return mp_obj_new_bool(false);
    }else{
        uint16_t button_mask = mp_obj_get_int(button_mask_u16);
        return mp_obj_new_bool(check_pressed(button_mask));
    }
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_io_check_pressed_obj, engine_io_check_pressed);


/*  --- doc ---
    NAME: check_just_changed
    ID: check_just_changed
    DESC: For checking buttons that were either just released or pressed. OR'ing together values means this returns true when all OR'ed buttons were just changed
    PARAM: [type=int]   [name=button_mask]  [value=single or OR'ed together enum/ints (e.g. 'engine_input.A | engine_input.B')]
    RETURN: True or False
*/
static mp_obj_t engine_io_check_just_changed(mp_obj_t button_mask_u16){
    if(engine_gui_is_gui_focused() == true){
        return mp_obj_new_bool(false);
    }else{
        uint16_t button_mask = mp_obj_get_int(button_mask_u16);
        return mp_obj_new_bool(check_just_changed(button_mask));
    }
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_io_check_just_changed_obj, engine_io_check_just_changed);


/*  --- doc ---
    NAME: check_just_pressed
    ID: check_just_pressed
    DESC: For checking buttons that were just pressed. OR'ing together values means this returns true when all OR'ed buttons were just pressed
    PARAM: [type=int]   [name=button_mask]  [value=single or OR'ed together enum/ints (e.g. 'engine_input.A | engine_input.B')]
    RETURN: True or False
*/
static mp_obj_t engine_io_check_just_pressed(mp_obj_t button_mask_u16){
    if(engine_gui_is_gui_focused() == true){
        return mp_obj_new_bool(false);
    }else{
        uint16_t button_mask = mp_obj_get_int(button_mask_u16);
        return mp_obj_new_bool(check_just_pressed(button_mask));
    }
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_io_check_just_pressed_obj, engine_io_check_just_pressed);


/*  --- doc ---
    NAME: check_just_released
    ID: check_just_released
    DESC: For checking buttons that were just released. OR'ing together values means this returns true when all OR'ed buttons were just released
    PARAM: [type=int]   [name=button_mask]  [value=single or OR'ed together enum/ints (e.g. 'engine_input.A | engine_input.B')]
    RETURN: True or False
*/
static mp_obj_t engine_io_check_just_released(mp_obj_t button_mask_u16){
    if(engine_gui_is_gui_focused() == true){
        return mp_obj_new_bool(false);
    }else{
        uint16_t button_mask = mp_obj_get_int(button_mask_u16);
        return mp_obj_new_bool(check_just_released(button_mask));
    }
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_io_check_just_released_obj, engine_io_check_just_released);


/*  --- doc ---
    NAME: rumble
    ID: rumble
    DESC: Run the internal vibration motor at some intensity
    PARAM: [type=float]   [name=intensity]  [value=0.0 ~ 1.0]
    RETURN: None
*/
static mp_obj_t engine_io_rumble(mp_obj_t intensity_obj){
    #ifdef __unix__
        // Nothing to do
    #else
        float intensity = mp_obj_get_float(intensity_obj);
        engine_io_rp3_rumble(intensity);
    #endif
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_io_rumble_obj, engine_io_rumble);


/*  --- doc ---
    NAME: gui_focused_toggle
    ID: gui_focused_toggle
    DESC: Toggle between button inputs being consumed by the game or gui elements.
    RETURN: bool
*/
static mp_obj_t engine_io_gui_focused_toggle(){
    return mp_obj_new_bool(engine_gui_toggle_focus());
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_io_gui_focused_toggle_obj, engine_io_gui_focused_toggle);


/*  --- doc ---
    NAME: gui_focused
    ID: gui_focused
    DESC: Get or set whether the GUI is focused.
    PARAM: [type=bool (optional)]   [name=gui_focused]  [value=True/False]
    RETURN: bool
*/
static mp_obj_t engine_io_gui_focused(size_t n_args, const mp_obj_t *args){
    if (n_args == 1) {
        engine_gui_set_focused(mp_obj_get_int(args[0]));
    }
    return mp_obj_new_bool(engine_gui_is_gui_focused());
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_io_gui_focused_obj, 0, 1, engine_io_gui_focused);


/*  --- doc ---
    NAME: focused_node
    ID: focused_node
    DESC: Get the currently focused node.
    RETURN: Node or None
*/
static mp_obj_t engine_io_focused_node(){
    engine_node_base_t *focused = engine_gui_get_focused();
    if(focused == NULL){
        return mp_const_none;
    }else{
        return focused;
    }
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_io_focused_node_obj, engine_io_focused_node);


/*  --- doc ---
    NAME: gui_toggle_button
    ID: gui_toggle_button
    DESC: Get or set the button that toggles GUI focus mode. None to disable.
    PARAM: [type=int|None (optional)]   [name=button]  [value=enum/int (e.g. 'engine_io.A')]
    RETURN: int
*/
static mp_obj_t engine_io_gui_toggle_button(size_t n_args, const mp_obj_t *args){
    if (n_args == 1) {
        gui_toggle_button = args[0] == mp_const_none ? 0 : mp_obj_get_int(args[0]);
    }
    return mp_obj_new_int(gui_toggle_button);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_io_gui_toggle_button_obj, 0, 1, engine_io_gui_toggle_button);


static mp_obj_t engine_io_battery_level(){
    #if defined(__arm__)
        // Read the 12-bit sample with ADC max ref voltage of 3.3V
        uint16_t battery_voltage_12_bit = adc_read();

        // Battery voltage is either 5V when charging or 4.2V to 2.75V on battery.
        // The input voltage we're measuring is before the LDO. The measured voltage
        // is dropped to below max readable reference voltage of 3.3V through 1/(1+1)
        // voltage divider (cutting it in half):
        // 5/2    = 2.5V
        // 4.2/2  = 2.1V    <- MAX
        // 3.3/2 = 1.65V    <- MIN
        float battery_half_voltage = battery_voltage_12_bit * ADC_CONV_FACTOR;

        // Map to the range we want to return.
        // Clamp since we only care showing between 0.0 and 1.0 for this function
        float battery_percentage = engine_math_map_clamp(battery_half_voltage, 1.65f, 2.1f, 0.0f, 1.0f);

        return mp_obj_new_float(battery_percentage);
    #endif

    return mp_obj_new_float(1.0f);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_io_battery_level_obj, engine_io_battery_level);


static mp_obj_t engine_io_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_io_module_init_obj, engine_io_module_init);


/*  --- doc ---
    NAME: engine_io
    ID: engine_io
    DESC: Module for checking button presses
    ATTR: [type=function]   [name={ref_link:check_pressed}]             [value=function]
    ATTR: [type=function]   [name={ref_link:check_just_changed}]        [value=function]
    ATTR: [type=function]   [name={ref_link:check_just_pressed}]        [value=function]
    ATTR: [type=function]   [name={ref_link:check_just_released}]       [value=function]
    ATTR: [type=function]   [name={ref_link:rumble}]                    [value=function]
    ATTR: [type=function]   [name={ref_link:gui_focused_toggle}]        [value=function]
    ATTR: [type=function]   [name={ref_link:gui_focused}]               [value=function]
    ATTR: [type=function]   [name={ref_link:focused_node}]              [value=function]
    ATTR: [type=function]   [name={ref_link:gui_toggle_button}]         [value=function]
    ATTR: [type=enum/int]   [name=A]                                    [value=0b0000000000000001]
    ATTR: [type=enum/int]   [name=B]                                    [value=0b0000000000000010]
    ATTR: [type=enum/int]   [name=DPAD_UP]                              [value=0b0000000000000100]
    ATTR: [type=enum/int]   [name=DPAD_DOWN]                            [value=0b0000000000001000]
    ATTR: [type=enum/int]   [name=DPAD_LEFT]                            [value=0b0000000000010000]
    ATTR: [type=enum/int]   [name=DPAD_RIGHT]                           [value=0b0000000000100000]
    ATTR: [type=enum/int]   [name=BUMPER_LEFT]                          [value=0b0000000001000000]
    ATTR: [type=enum/int]   [name=BUMPER_RIGHT]                         [value=0b0000000010000000]
    ATTR: [type=enum/int]   [name=MENU]                                 [value=0b0000000100000000]
*/
static const mp_rom_map_elem_t engine_io_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_io) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_io_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_check_pressed), MP_ROM_PTR(&engine_io_check_pressed_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_check_just_changed), MP_ROM_PTR(&engine_io_check_just_changed_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_check_just_pressed), MP_ROM_PTR(&engine_io_check_just_pressed_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_check_just_released), MP_ROM_PTR(&engine_io_check_just_released_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rumble), MP_ROM_PTR(&engine_io_rumble_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gui_focused), MP_ROM_PTR(&engine_io_gui_focused_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gui_focused_toggle), MP_ROM_PTR(&engine_io_gui_focused_toggle_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_battery_level), MP_ROM_PTR(&engine_io_battery_level_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_focused_node), MP_ROM_PTR(&engine_io_focused_node_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gui_toggle_button), MP_ROM_PTR(&engine_io_gui_toggle_button_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_A), MP_ROM_INT(BUTTON_A) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_B), MP_ROM_INT(BUTTON_B) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DPAD_UP), MP_ROM_INT(BUTTON_DPAD_UP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DPAD_DOWN), MP_ROM_INT(BUTTON_DPAD_DOWN) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DPAD_LEFT), MP_ROM_INT(BUTTON_DPAD_LEFT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DPAD_RIGHT), MP_ROM_INT(BUTTON_DPAD_RIGHT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_BUMPER_LEFT), MP_ROM_INT(BUTTON_BUMPER_LEFT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_BUMPER_RIGHT), MP_ROM_INT(BUTTON_BUMPER_RIGHT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_MENU), MP_ROM_INT(BUTTON_MENU) },
};
static MP_DEFINE_CONST_DICT (mp_module_engine_io_globals, engine_io_globals_table);


const mp_obj_module_t engine_io_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_io_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_io, engine_io_user_cmodule);
