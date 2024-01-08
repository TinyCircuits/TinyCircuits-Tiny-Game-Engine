#include "py/obj.h"
#include "engine_input_common.h"

#ifdef __unix__
    #include "engine_input_sdl.h"
#else
    #include "engine_input_rp3.h"
#endif


void engine_input_setup(){
    #ifdef __unix__
        // Nothing to do
    #else
        engine_input_rp3_setup();
    #endif
}


// Update 'pressed_buttons' (usually called once per game loop)
void engine_input_update_pressed_buttons(){
    #ifdef __unix__
        engine_input_sdl_update_pressed_mask();
    #else
        engine_input_rp3_update_pressed_mask();
    #endif
}


STATIC mp_obj_t engine_input_is_A_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_A));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_A_pressed_obj, engine_input_is_A_pressed);


STATIC mp_obj_t engine_input_is_B_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_B));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_B_pressed_obj, engine_input_is_B_pressed);


STATIC mp_obj_t engine_input_is_DPAD_UP_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_DPAD_UP));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_DPAD_UP_pressed_obj, engine_input_is_DPAD_UP_pressed);


STATIC mp_obj_t engine_input_is_DPAD_DOWN_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_DPAD_DOWN));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_DPAD_DOWN_pressed_obj, engine_input_is_DPAD_DOWN_pressed);


STATIC mp_obj_t engine_input_is_DPAD_LEFT_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_DPAD_LEFT));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_DPAD_LEFT_pressed_obj, engine_input_is_DPAD_LEFT_pressed);


STATIC mp_obj_t engine_input_is_DPAD_RIGHT_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_DPAD_RIGHT));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_DPAD_RIGHT_pressed_obj, engine_input_is_DPAD_RIGHT_pressed);


STATIC mp_obj_t engine_input_is_BUMPER_LEFT_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_BUMPER_LEFT));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_BUMPER_LEFT_pressed_obj, engine_input_is_BUMPER_LEFT_pressed);


STATIC mp_obj_t engine_input_is_BUMPER_RIGHT_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_BUMPER_RIGHT));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_BUMPER_RIGHT_pressed_obj, engine_input_is_BUMPER_RIGHT_pressed);


STATIC mp_obj_t engine_input_is_MENU_pressed(){
    return mp_obj_new_bool(BIT_GET(engine_input_pressed_buttons, BUTTON_MENU));
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_input_is_MENU_pressed_obj, engine_input_is_MENU_pressed);


// Module attributes
STATIC const mp_rom_map_elem_t engine_input_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_input) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_a_pressed), (mp_obj_t)&engine_input_is_A_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_b_pressed), (mp_obj_t)&engine_input_is_B_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_dpad_up_pressed), (mp_obj_t)&engine_input_is_DPAD_UP_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_dpad_down_pressed), (mp_obj_t)&engine_input_is_DPAD_DOWN_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_dpad_left_pressed), (mp_obj_t)&engine_input_is_DPAD_LEFT_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_dpad_right_pressed), (mp_obj_t)&engine_input_is_DPAD_RIGHT_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_bumper_left_pressed), (mp_obj_t)&engine_input_is_BUMPER_LEFT_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_bumper_right_pressed), (mp_obj_t)&engine_input_is_BUMPER_RIGHT_pressed_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_is_menu_pressed), (mp_obj_t)&engine_input_is_MENU_pressed_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_input_globals, engine_input_globals_table);

const mp_obj_module_t engine_input_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_input_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_input, engine_input_user_cmodule);