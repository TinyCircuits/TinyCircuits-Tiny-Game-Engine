#include <stdio.h>
#include "py/obj.h"
#include "py/objtype.h"
#include "py/runtime.h"
#include "py/builtin.h"



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
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually return from this function)
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

// Module attributes
STATIC const mp_map_elem_t engine_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_simpleclass) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_BaseNode), (mp_obj_t)&engine_base_node_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_start), (mp_obj_t)&engine_start_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_globals, engine_globals_table);

const mp_obj_module_t engine_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine, engine_user_cmodule);






// typedef struct _cb_finalizer_t {
//     mp_obj_base_t base;
//     mp_obj_t fun;
// } cb_finalizer_t;

// extern const mp_obj_type_t mp_type_cb_finalizer;

// STATIC mp_obj_t new_cb_finalizer(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
//     mp_arg_check_num(n_args, n_kw, 1, 1, false);
//     cb_finalizer_t *o = m_new_obj_with_finaliser(cb_finalizer_t);
//     o->base.type = &mp_type_cb_finalizer;
//     o->fun = args[0];
//     return o;
// }

// STATIC mp_obj_t cb_finalizer_del(mp_obj_t self_in) {
//     return mp_call_function_0(((cb_finalizer_t *)self_in)->fun);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(cb_finalizer_del_obj, cb_finalizer_del);

// STATIC void cb_finalizer_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
//     if (dest[0] == MP_OBJ_NULL && attr == MP_QSTR___del__) {
//         dest[0] = MP_OBJ_FROM_PTR(&cb_finalizer_del_obj);
//         dest[1] = self_in;
//     }
// }

// const mp_obj_type_t mp_type_cb_finalizer = {
//     {&mp_type_type},
//     .name = MP_QSTR_cb_finalizer,
//     .make_new = new_cb_finalizer,
//     .attr = cb_finalizer_attr,
// };