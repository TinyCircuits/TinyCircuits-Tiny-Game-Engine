#include "py/obj.h"

#include "debug_print.h"


/*  --- doc ---
    NAME: disable_all
    DESC: Disables all debug levels/outputs
    RETURN: None
*/ 
STATIC mp_obj_t engine_debug_disable_all(){
    DEBUG_INFO_ENABLED = false;
    DEBUG_WARNINGS_ENABLED = false;
    DEBUG_ERRORS_ENABLED = false;
    DEBUG_PERFORMANCE_ENABLED = false;
    ENGINE_FORCE_PRINTF("Disabled all debug prints");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_debug_disable_all_obj, engine_debug_disable_all);


/*  --- doc ---
    NAME: enable_all
    DESC: Enables all debug levels/outputs
    RETURN: None
*/ 
STATIC mp_obj_t engine_debug_enable_all(){
    DEBUG_INFO_ENABLED = true;
    DEBUG_WARNINGS_ENABLED = true;
    DEBUG_ERRORS_ENABLED = true;
    DEBUG_PERFORMANCE_ENABLED = true;
    ENGINE_FORCE_PRINTF("Enabled all debug prints");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_debug_enable_all_obj, engine_debug_enable_all);


/*  --- doc ---
    NAME: enable_setting
    DESC: Enables a debug level/output
    PARAM: [type=enum/int]   [name=debug_setting]   [value=enum/int 0 ~ 3]
    RETURN: None
*/ 
STATIC mp_obj_t engine_debug_enable_setting(mp_obj_t debug_setting){
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
MP_DEFINE_CONST_FUN_OBJ_1(engine_debug_enable_setting_obj, engine_debug_enable_setting);


/*  --- doc ---
    NAME: engine_debug
    DESC: Module for what types of information gets printed from the engine
    ATTR: [type=function]   [name={ref_link:enable_all}]        [value=function] 
    ATTR: [type=function]   [name={ref_link:disable_all}]       [value=function]
    ATTR: [type=function]   [name={ref_link:enable_setting}]    [value=function]
    ATTR: [type=enum/int]   [name=info]                         [value=0]
    ATTR: [type=enum/int]   [name=warnings]                     [value=1]
    ATTR: [type=enum/int]   [name=errors]                       [value=2]
    ATTR: [type=enum/int]   [name=performance]                  [value=3]
*/ 
STATIC const mp_rom_map_elem_t engine_debug_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_debug) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_enable_all), (mp_obj_t)&engine_debug_enable_all_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_disable_all), (mp_obj_t)&engine_debug_disable_all_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_enable_setting), (mp_obj_t)&engine_debug_enable_setting_obj },
    { MP_ROM_QSTR(MP_QSTR_info), MP_ROM_INT(DEBUG_SETTING_INFO) },
    { MP_ROM_QSTR(MP_QSTR_warnings), MP_ROM_INT(DEBUG_SETTING_WARNINGS) },
    { MP_ROM_QSTR(MP_QSTR_errors), MP_ROM_INT(DEBUG_SETTING_ERRORS) },
    { MP_ROM_QSTR(MP_QSTR_performance), MP_ROM_INT(DEBUG_SETTING_PERFORMANCE) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_debug_globals, engine_debug_globals_table);

const mp_obj_module_t engine_debug_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_debug_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_debug, engine_debug_user_cmodule);