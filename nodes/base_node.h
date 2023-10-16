#ifndef BASE_NODE_H
#define BASE_NODE_H

#include "py/obj.h"
#include "py/objtype.h"
#include "py/runtime.h"
#include "py/builtin.h"

#include "engine.h"
#include "utility/debug_print.h"


// Class type
typedef struct {
    mp_obj_base_t base;
    mp_obj_t *child;
    object_list_node *object_list_node;
}engine_base_node_class_obj_t;

const mp_obj_type_t engine_base_node_class_type;



// Class required functions
STATIC void base_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    ENGINE_INFO_PRINTF("print(): BaseNode");
}

STATIC mp_obj_t base_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    ENGINE_INFO_PRINTF("New BaseNode");
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_base_node_class_obj_t *self = m_new_obj_with_finaliser(engine_base_node_class_obj_t);

    ENGINE_INFO_PRINTF("Registering subclass with BaseNode and adding self to engine active objects");
    self->child = MP_OBJ_TO_PTR(args[0]);
    self->object_list_node = object_list_add_obj(&engine_objects, self);

    self->base.type = &engine_base_node_class_type;

    return self;
}


// Class methods

STATIC mp_obj_t base_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("BaseNode deleted (garbage collected, removing self from active engine objects)");

    engine_base_node_class_obj_t *self = ((engine_base_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    object_list_del_list_node(&engine_objects, self->object_list_node);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_del_obj, base_node_class_del);


STATIC mp_obj_t base_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_tick_obj, base_node_class_tick);


// Class attributes
STATIC const mp_rom_map_elem_t base_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&base_node_class_del_obj) },
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

#endif  // BASE_NODE_H