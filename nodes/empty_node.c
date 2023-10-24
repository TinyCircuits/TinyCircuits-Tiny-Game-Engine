#include "empty_node.h"

#include "node_types.h"
#include "utility/debug_print.h"
#include "../engine_object_layers.h"

// Class required functions
STATIC void empty_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): EmptyNode");
}

STATIC mp_obj_t empty_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New EmptyNode");

    // Check that there's an argument that's hopefully a reference to the inheriting subclass
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_empty_node_class_obj_t *self = m_new_obj_with_finaliser(engine_empty_node_class_obj_t);
    self->base.type = &engine_empty_node_class_type;
    self->node_base.layer = 0;
    self->node_base.type = NODE_TYPE_EMPTY;
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);
    node_base_set_if_visible(&self->node_base, true);
    node_base_set_if_disabled(&self->node_base, false);
    node_base_set_if_just_added(&self->node_base, true);

    // Cache lookup results of 'tick()' function on this node
    // instance so that the main engine loop can call it quickly
    mp_load_method(MP_OBJ_TO_PTR(args[0]), MP_QSTR_tick, self->tick_dest);

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t empty_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("EmptyNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_empty_node_class_obj_t *self = ((engine_empty_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(empty_node_class_del_obj, empty_node_class_del);


STATIC mp_obj_t empty_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("EmptyNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(empty_node_class_tick_obj, empty_node_class_tick);


STATIC mp_obj_t empty_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_empty_node_class_obj_t *self = ((engine_empty_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    engine_remove_object_from_layer(self->node_base.object_list_node, self->node_base.layer);
    self->node_base.layer = (uint16_t)mp_obj_get_int(layer);
    self->node_base.object_list_node = engine_add_object_to_layer(self, self->node_base.layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(empty_node_class_set_layer_obj, empty_node_class_set_layer);


// Class attributes
STATIC const mp_rom_map_elem_t empty_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&empty_node_class_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick), MP_ROM_PTR(&empty_node_class_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_layer), MP_ROM_PTR(&empty_node_class_set_layer_obj) },
};


// Class init
STATIC MP_DEFINE_CONST_DICT(empty_node_class_locals_dict, empty_node_class_locals_dict_table);

const mp_obj_type_t engine_empty_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_EmptyNode,
    .print = empty_node_class_print,
    .make_new = empty_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = NULL,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&empty_node_class_locals_dict,
};