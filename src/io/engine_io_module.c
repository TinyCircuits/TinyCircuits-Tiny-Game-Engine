#include "py/obj.h"
#include "engine_io_buttons.h"
#include "engine_gui.h"
#include "engine_main.h"
#include "math/engine_math.h"
#include "utility/engine_defines.h"


button_class_obj_t* gui_toggle_button = &BUTTON_MENU;


void engine_io_setup(){
    #ifdef __unix__
        // Nothing to do
    #else
        engine_io_rp3_setup();
    #endif
}


void engine_io_tick(){
    buttons_update_state();
}


button_class_obj_t* engine_io_get_gui_toggle_button(){
    return gui_toggle_button;
}


void engine_io_reset_gui_toggle_button(){
    gui_toggle_button = &BUTTON_MENU;
}


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
    DESC: Toggle between button inputs being consumed by the game or gui elements. Also reset all buttons (see {ref_link:reset_all_buttons})).
    RETURN: bool
*/
static mp_obj_t engine_io_gui_focused_toggle(){
    bool gui_focused = engine_gui_toggle_focus();
    buttons_reset_all();
    return mp_obj_new_bool(gui_focused);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_io_gui_focused_toggle_obj, engine_io_gui_focused_toggle);


/*  --- doc ---
    NAME: gui_focused
    ID: gui_focused
    DESC: Get or set whether the GUI is focused. If set, also reset all buttons (see {ref_link:reset_all_buttons})).
    PARAM: [type=bool (optional)]   [name=gui_focused]  [value=True/False]
    RETURN: bool
*/
static mp_obj_t engine_io_gui_focused(size_t n_args, const mp_obj_t *args){
    if (n_args == 1) {
        engine_gui_set_focused(mp_obj_get_int(args[0]));
        buttons_reset_all();
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
        if (args[0] == mp_const_none) {
            gui_toggle_button = NULL;
        } else if (mp_obj_is_type(args[0], &button_class_type)) {
            gui_toggle_button = MP_OBJ_TO_PTR(args[0]);
        } else {
            mp_raise_TypeError(MP_ERROR_TEXT("Expected Button or None"));
        }
    }
    return MP_OBJ_FROM_PTR(gui_toggle_button);
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_io_gui_toggle_button_obj, 0, 1, engine_io_gui_toggle_button);


/*  --- doc ---
    NAME: battery_level
    ID: battery_level
    DESC: Get the battery level as a float between 0.0 and 1.0
    RETURN: float

*/
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
    NAME: reset_all_buttons
    ID: reset_all_buttons
    DESC: Treat each button as if released, until it is next pressed. This is very useful after changing app "screen", e.g. entering submenu.
    RETURN: None
*/
static mp_obj_t engine_io_reset_all_buttons(){
    buttons_reset_all();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(buttons_reset_all_obj, engine_io_reset_all_buttons);


/*  --- doc ---
    NAME: engine_io
    ID: engine_io
    DESC: Module for checking button presses
    ATTR: [type=function]            [name={ref_link:rumble}]                   [value=function]
    ATTR: [type=function]            [name={ref_link:gui_focused}]              [value=getter/setter function]
    ATTR: [type=function]            [name={ref_link:gui_focused_toggle}]       [value=function]
    ATTR: [type=function]            [name={ref_link:battery_level}]            [value=function]
    ATTR: [type=function]            [name={ref_link:focused_node}]             [value=function]
    ATTR: [type=function]            [name={ref_link:gui_toggle_button}]        [value=setter/setter function]
    ATTR: [type=type]                [name={ref_link:Button}]                   [value=the Button class]
    ATTR: [type={ref_link:Button}]   [name=UP]                                  [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=DOWN]                                [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=LEFT]                                [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=RIGHT]                               [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=A]                                   [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=B]                                   [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=LB]                                  [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=RB]                                  [value=the button object]
    ATTR: [type={ref_link:Button}]   [name=MENU]                                [value=the button object]
    ATTR: [type=function]            [name={ref_link:reset_all_buttons}]        [value=function]
*/
static const mp_rom_map_elem_t engine_io_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_io) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), MP_ROM_PTR(&engine_io_module_init_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rumble), MP_ROM_PTR(&engine_io_rumble_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gui_focused), MP_ROM_PTR(&engine_io_gui_focused_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gui_focused_toggle), MP_ROM_PTR(&engine_io_gui_focused_toggle_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_battery_level), MP_ROM_PTR(&engine_io_battery_level_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_focused_node), MP_ROM_PTR(&engine_io_focused_node_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gui_toggle_button), MP_ROM_PTR(&engine_io_gui_toggle_button_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Button), MP_ROM_PTR(&button_class_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_UP), MP_ROM_PTR(&BUTTON_DPAD_UP) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_DOWN), MP_ROM_PTR(&BUTTON_DPAD_DOWN) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_LEFT), MP_ROM_PTR(&BUTTON_DPAD_LEFT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RIGHT), MP_ROM_PTR(&BUTTON_DPAD_RIGHT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_A), MP_ROM_PTR(&BUTTON_A) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_B), MP_ROM_PTR(&BUTTON_B) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_LB), MP_ROM_PTR(&BUTTON_BUMPER_LEFT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_RB), MP_ROM_PTR(&BUTTON_BUMPER_RIGHT) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_MENU), MP_ROM_PTR(&BUTTON_MENU) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_reset_all_buttons), MP_ROM_PTR(&buttons_reset_all_obj) },
};
static MP_DEFINE_CONST_DICT (mp_module_engine_io_globals, engine_io_globals_table);


const mp_obj_module_t engine_io_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_io_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_io, engine_io_user_cmodule);
