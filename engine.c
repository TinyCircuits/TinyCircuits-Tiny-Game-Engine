#include <stdio.h>
#include "py/obj.h"
#include "py/objtype.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "engine.h"
#include "utility/debug_print.h"
#include "nodes/base_node.h"





// ### MODULE ###

// Module functions
STATIC mp_obj_t engine_start(){
    ENGINE_INFO_PRINTF("Engine starting...");

    object_list_node *current_node = NULL;
    mp_obj_t dest[2];

    while(true){
        current_node = engine_objects.root.next;

        while(current_node != NULL){
            engine_base_node_class_obj_t *current_node_base_class_instance = ((engine_base_node_class_obj_t*)current_node->object);

            mp_load_method(current_node_base_class_instance->child, MP_QSTR_tick, dest);
            mp_call_method_n_kw(0, 0, dest);

            current_node = current_node->next;
        }
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_start_obj, engine_start);


STATIC mp_obj_t engine_set_debug_level(mp_obj_t debug_level){
    // Translate the debug level and check if inbounds
    uint8_t engine_debug_level = mp_obj_get_int(debug_level);
    if(engine_debug_level > DEBUG_PRINT_LEVEL_INFO){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Tried to set debug level to unknown value"));
    }

    // Set level and give info print
    ENGINE_DEBUG_PRINT_LEVEL = engine_debug_level;

    if(engine_debug_level == DEBUG_PRINT_LEVEL_NONE) {ENGINE_FORCE_PRINTF("Set engine debug level to NONE: the engine will not print errors, warnings, or info messages (exceptions will still occur)")};
    if(engine_debug_level == DEBUG_PRINT_LEVEL_ALL) {ENGINE_FORCE_PRINTF("Set engine debug level to ALL: the engine will print all errors, warnings, and info messages")};
    if(engine_debug_level == DEBUG_PRINT_LEVEL_WARNINGS) {ENGINE_FORCE_PRINTF("Set engine debug level to WARNINGS: the engine will only print warning and info messages")};
    if(engine_debug_level == DEBUG_PRINT_LEVEL_INFO) {ENGINE_FORCE_PRINTF("Set engine debug level to INFO: the engine will only print info messages")};

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_set_debug_level_obj, engine_set_debug_level);



// Module attributes
STATIC const mp_map_elem_t engine_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_BaseNode), (mp_obj_t)&engine_base_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_start_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_debug_print_level), (mp_obj_t)&engine_set_debug_level_obj },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_none), MP_ROM_INT(DEBUG_PRINT_LEVEL_NONE) },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_all), MP_ROM_INT(DEBUG_PRINT_LEVEL_ALL) },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_warnings), MP_ROM_INT(DEBUG_PRINT_LEVEL_WARNINGS) },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_info), MP_ROM_INT(DEBUG_PRINT_LEVEL_INFO) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_globals, engine_globals_table);

const mp_obj_module_t engine_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine, engine_user_cmodule);