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


STATIC mp_obj_t engine_input_check_pressed(mp_obj_t button_mask_u16){
    uint16_t button_mask = mp_obj_get_int(button_mask_u16);

    // Check that the OR'ed button mask exactly matches the
    // internally updated currently pressed button mask exactly
    return mp_obj_new_bool(engine_input_pressed_buttons == button_mask);
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_input_check_pressed_obj, engine_input_check_pressed);


// Module attributes
STATIC const mp_rom_map_elem_t engine_input_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_input) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_check_pressed), (mp_obj_t)&engine_input_check_pressed_obj },

    { MP_ROM_QSTR(MP_QSTR_A), MP_ROM_INT(BUTTON_A) },
    { MP_ROM_QSTR(MP_QSTR_B), MP_ROM_INT(BUTTON_B) },
    { MP_ROM_QSTR(MP_QSTR_DPAD_UP), MP_ROM_INT(BUTTON_DPAD_UP) },
    { MP_ROM_QSTR(MP_QSTR_DPAD_DOWN), MP_ROM_INT(BUTTON_DPAD_DOWN) },
    { MP_ROM_QSTR(MP_QSTR_DPAD_LEFT), MP_ROM_INT(BUTTON_DPAD_LEFT) },
    { MP_ROM_QSTR(MP_QSTR_DPAD_RIGHT), MP_ROM_INT(BUTTON_DPAD_RIGHT) },
    { MP_ROM_QSTR(MP_QSTR_BUMPER_LEFT), MP_ROM_INT(BUTTON_BUMPER_LEFT) },
    { MP_ROM_QSTR(MP_QSTR_BUMPER_RIGHT), MP_ROM_INT(BUTTON_BUMPER_RIGHT) },
    { MP_ROM_QSTR(MP_QSTR_MENU), MP_ROM_INT(BUTTON_MENU) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_input_globals, engine_input_globals_table);

const mp_obj_module_t engine_input_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_input_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_input, engine_input_user_cmodule);