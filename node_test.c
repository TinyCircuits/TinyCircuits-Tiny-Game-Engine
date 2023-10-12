#include <stdio.h>
#include "py/obj.h"
#include "py/objtype.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "utility/debug_print.h"



// ### CLASS ###

// Class type
typedef struct _engine_base_node_class_obj_t {
    mp_obj_base_t base;
}engine_base_node_class_obj_t;

const mp_obj_type_t engine_base_node_class_type;

uint16_t object_count = 0;
mp_obj_t *objects[10];



// Class required functions
STATIC void base_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    mp_print_str(print, "I am the base node class");
}

STATIC mp_obj_t base_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_print_str(&mp_sys_stdout_print, "base node new\n");
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_base_node_class_obj_t *self = m_new_obj_with_finaliser(engine_base_node_class_obj_t);
    self->base.type = &engine_base_node_class_type;

    return self;
}


// Class methods
STATIC mp_obj_t base_node_class_init(mp_obj_t self_in) {
    mp_print_str(&mp_sys_stdout_print, "base node init\n");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_init_obj, base_node_class_init);

STATIC mp_obj_t base_node_class_del(mp_obj_t self_in) {
    mp_print_str(&mp_sys_stdout_print, "base node DELETE\n");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_del_obj, base_node_class_del);

STATIC mp_obj_t base_node_class_register(mp_obj_t self_in, mp_obj_t child_in) {
    mp_print_str(&mp_sys_stdout_print, "Register node\n");

    objects[object_count] = MP_OBJ_TO_PTR(child_in);
    object_count++;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(base_node_class_register_obj, base_node_class_register);

STATIC mp_obj_t base_node_class_tick(mp_obj_t self_in) {
    mp_print_str(&mp_sys_stdout_print, "override me please and thanks\n");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_tick_obj, base_node_class_tick);


// Class attributes
STATIC const mp_rom_map_elem_t base_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&base_node_class_init_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&base_node_class_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_register), MP_ROM_PTR(&base_node_class_register_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick), MP_ROM_PTR(&base_node_class_tick_obj) },
};

// Class init
STATIC MP_DEFINE_CONST_DICT(base_node_class_locals_dict, base_node_class_locals_dict_table);

const mp_obj_type_t engine_base_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_BaseNode,
    .print = base_node_class_print,
    .make_new = base_node_class_new,
    .locals_dict = (mp_obj_dict_t*)&base_node_class_locals_dict,
};





// ### MODULE ###

// Module functions
STATIC mp_obj_t engine_start(){
    ENGINE_ERROR_PRINTF("test");

    mp_printf(&mp_sys_stdout_print, "%d\n", sizeof(objects[0])*10);

    mp_print_str(&mp_sys_stdout_print, "engine start\n");
    mp_obj_print_helper(&mp_sys_stdout_print, mp_obj_new_int(object_count), PRINT_REPR);
    mp_print_str(&mp_sys_stdout_print, "\n");
    // while(true){
        for(uint8_t iox=0; iox<object_count; iox++){
            mp_obj_t dest[2];
            mp_load_method(objects[iox], MP_QSTR_tick, dest);
            mp_call_method_n_kw(0, 0, dest);
        }
    // }
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
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_simpleclass) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_BaseNode), (mp_obj_t)&engine_base_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_start_obj },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_none), MP_ROM_INT(DEBUG_PRINT_LEVEL_NONE) },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_all), MP_ROM_INT(DEBUG_PRINT_LEVEL_ALL) },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_warnings), MP_ROM_INT(DEBUG_PRINT_LEVEL_WARNINGS) },
    { MP_ROM_QSTR(MP_QSTR_debug_print_level_info), MP_ROM_INT(DEBUG_PRINT_LEVEL_INFO) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_debug_print_level), (mp_obj_t)&engine_set_debug_level_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_globals, engine_globals_table);

const mp_obj_module_t engine_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine, engine_user_cmodule);