#include "base_node.h"
#include "utility/debug_print.h"
#include "../engine_object_layers.h"

// Class required functions
STATIC void base_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): BaseNode");
}

STATIC mp_obj_t base_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New BaseNode");
    mp_arg_check_num(n_args, n_kw, 1, 1, true);

    // How to make __del__ get called when object is garbage collected: https://github.com/micropython/micropython/issues/1878
    // Why it might get called early: https://forum.micropython.org/viewtopic.php?t=1405 (make sure the object is actually returned from this function)
    engine_base_node_class_obj_t *self = m_new_obj_with_finaliser(engine_base_node_class_obj_t);

    self->base.type = &engine_base_node_class_type;

    ENGINE_INFO_PRINTF("Registering subclass with BaseNode and adding self to engine active objects");
    self->child = MP_OBJ_TO_PTR(args[0]);

    // By default all objects go to layer 0 for rendering and execution order
    self->layer = 0;

    self->object_list_node = engine_add_object_to_layer(self, self->layer);

    // Cache lookup results of 'tick()' function on this node
    // instance so that the main engine loop can call it quickly
    mp_load_method(self->child, MP_QSTR_tick, self->tick_dest);

    self->visible = true;
    self->disabled = false;

    // Set so that engine can set it false, loop, then call its callbacks
    self->just_added = true;

    return self;
}


// Class methods

STATIC mp_obj_t base_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("BaseNode deleted (garbage collected, removing self from active engine objects)");

    engine_base_node_class_obj_t *self = ((engine_base_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    engine_remove_object_from_layer(self->object_list_node, self->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_del_obj, base_node_class_del);


STATIC mp_obj_t base_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(base_node_class_tick_obj, base_node_class_tick);


STATIC mp_obj_t base_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_base_node_class_obj_t *self = ((engine_base_node_class_obj_t*)MP_OBJ_TO_PTR(self_in));
    engine_remove_object_from_layer(self->object_list_node, self->layer);
    self->layer = (uint16_t)mp_obj_get_int(layer);
    self->object_list_node = engine_add_object_to_layer(self, self->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(base_node_class_set_layer_obj, base_node_class_set_layer);


// Class attributes
STATIC const mp_rom_map_elem_t base_node_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&base_node_class_del_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick), MP_ROM_PTR(&base_node_class_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_layer), MP_ROM_PTR(&base_node_class_set_layer_obj) },
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