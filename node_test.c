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
    engine_base_node_class_obj_t *self = m_new_obj(engine_base_node_class_obj_t);
    self->base.type = &engine_base_node_class_type;

    return objects[object_count];
}


// Class methods
STATIC mp_obj_t base_node_class_init(mp_obj_t self_in) {
    mp_print_str(&mp_sys_stdout_print, "base node init\n");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_init_obj, base_node_class_init);

STATIC mp_obj_t base_node_class_del(mp_obj_t self_in) {
    mp_print_str(&mp_sys_stdout_print, "base node del\n");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_del_obj, base_node_class_del);

STATIC mp_obj_t base_node_class_register(mp_obj_t self_in, mp_obj_t child_in) {
    mp_print_str(&mp_sys_stdout_print, "Register\n");

    objects[object_count] = MP_OBJ_TO_PTR(child_in);
    object_count++;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(base_node_class_register_obj, base_node_class_register);

STATIC mp_obj_t base_node_class_tick() {
    mp_print_str(&mp_sys_stdout_print, "override me please and thanks\n");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(base_node_class_tick_obj, base_node_class_tick);


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
    for(uint8_t iox=0; iox<object_count; iox++){

        mp_obj_t dest[2];
        mp_load_method(objects[iox], MP_QSTR_tick, dest);
        mp_call_method_n_kw(0, 0, dest);
        // mp_call_function_n_kw(dest[], 0, 0, MP_OBJ_NULL);
        // return mp_obj_get_int(mp_call_method_n_kw(0, 0, dest));


        // mp_obj_t gc_tick_fn = mp_load_attr(objects[iox], MP_QSTR_tick);
        // if (gc_tick_fn) {
        // //     mp_obj_print_helper(&mp_sys_stdout_print, "Found tick function", PRINT_REPR);
        // //     mp_print_str(&mp_sys_stdout_print, "\n");
        // //     // mp_call_function_0(gc_disable_fn);
        // }


        // mp_obj_instance_t *self = MP_OBJ_TO_PTR(objects[iox]);
        // mp_obj_print_helper(&mp_sys_stdout_print, MP_OBJ_TO_PTR(self->subobj[4]), PRINT_REPR);
        // mp_print_str(&mp_sys_stdout_print, "\n");
        // mp_call_function_n_kw(self->subobj[4], 0, 0, MP_OBJ_NULL);



        // // look for 'tick' function
        // mp_obj_t tick_fn[2] = {MP_OBJ_NULL};
        // struct class_lookup_data lookup = {
        //     .obj = NULL,
        //     .attr = MP_QSTR_tick,
        //     .meth_offset = offsetof(mp_obj_type_t, base_node_class_tick),
        //     .dest = test_fn,
        //     .is_type = false,
        // };
        // mp_obj_class_lookup(&lookup, objects[iox]);

        // if (tick_fn[0] == MP_OBJ_NULL || init_fn[0] == MP_OBJ_SENTINEL) {

        // }

        // mp_obj_instance_call(MP_OBJ_FROM_PTR(objects[iox]), 0, 0, NULL);
        // mp_call_method_self_n_kw();
        // MP_OBJ_FROM_PTR(self)
    }
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