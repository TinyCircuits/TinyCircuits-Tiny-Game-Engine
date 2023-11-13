#include "py/obj.h"

#include "debug_print.h"


STATIC mp_obj_t engine_debug_disable_all(){
    DEBUG_INFO_ENABLED = false;
    DEBUG_WARNINGS_ENABLED = false;
    DEBUG_ERRORS_ENABLED = false;
    DEBUG_PERFORMANCE_ENABLED = false;
    ENGINE_FORCE_PRINTF("Disabled all debug prints");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_debug_disable_all_obj, engine_debug_disable_all);


STATIC mp_obj_t engine_debug_enable_all(){
    DEBUG_INFO_ENABLED = true;
    DEBUG_WARNINGS_ENABLED = true;
    DEBUG_ERRORS_ENABLED = true;
    DEBUG_PERFORMANCE_ENABLED = true;
    ENGINE_FORCE_PRINTF("Enabled all debug prints");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_debug_enable_all_obj, engine_debug_enable_all);


STATIC mp_obj_t engine_enable_debug_setting(mp_obj_t debug_setting){
    // Translate the debug level and check if inbounds
    uint8_t engine_debug_setting = mp_obj_get_int(debug_setting);

    switch(engine_debug_setting){
        case DEBUG_SETTING_INFO:
            DEBUG_INFO_ENABLED = true;
            ENGINE_FORCE_PRINTF("Enabled info debug prints");
        break;
        case DEBUG_SETTING_WARNINGS: 
            DEBUG_WARNINGS_ENABLED = true;
            ENGINE_FORCE_PRINTF("Enabled warning debug prints");
        break;
        case DEBUG_SETTING_ERRORS:
            DEBUG_ERRORS_ENABLED = true;
            ENGINE_FORCE_PRINTF("Enabled error debug prints");
        break;
        case DEBUG_SETTING_PERFORMANCE:
            DEBUG_PERFORMANCE_ENABLED = true;
            ENGINE_FORCE_PRINTF("Enabled performance debug prints");
        break;
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_enable_debug_setting_obj, engine_enable_debug_setting);


// Module attributes
STATIC const mp_rom_map_elem_t engine_debug_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_debug) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_debug_enable_all), (mp_obj_t)&engine_debug_enable_all_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_debug_disable_all), (mp_obj_t)&engine_debug_disable_all_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_debug_enable_setting), (mp_obj_t)&engine_enable_debug_setting_obj },
    { MP_ROM_QSTR(MP_QSTR_debug_setting_info), MP_ROM_INT(DEBUG_SETTING_INFO) },
    { MP_ROM_QSTR(MP_QSTR_debug_setting_warnings), MP_ROM_INT(DEBUG_SETTING_WARNINGS) },
    { MP_ROM_QSTR(MP_QSTR_debug_setting_errors), MP_ROM_INT(DEBUG_SETTING_ERRORS) },
    { MP_ROM_QSTR(MP_QSTR_debug_setting_performance), MP_ROM_INT(DEBUG_SETTING_PERFORMANCE) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_debug_globals, engine_debug_globals_table);

const mp_obj_module_t engine_debug_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_debug_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_debug, engine_debug_user_cmodule);