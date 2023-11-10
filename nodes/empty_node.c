#include "empty_node.h"

#include "node_types.h"
#include "utility/debug_print.h"
#include "../engine_object_layers.h"

// Class required functions
STATIC void empty_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): EmptyNode");
}


STATIC mp_obj_t empty_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("EmptyNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(empty_node_class_tick_obj, empty_node_class_tick);


STATIC mp_obj_t empty_node_class_draw(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("EmptyNode: Draw function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(empty_node_class_draw_obj, empty_node_class_draw);


STATIC mp_obj_t empty_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New EmptyNode");

    engine_empty_node_common_data_t *common_data = malloc(sizeof(engine_empty_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_empty_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_EMPTY;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;
        node_base->node = NULL;
        common_data->tick_cb = MP_OBJ_FROM_PTR(&empty_node_class_tick_obj);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];

        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&empty_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to EmptyNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC mp_obj_t empty_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("EmptyNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    free(node_base->node_common_data);
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);

    return mp_const_none;;
}
MP_DEFINE_CONST_FUN_OBJ_1(empty_node_class_del_obj, empty_node_class_del);


STATIC mp_obj_t empty_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_node_base_t *node_base = self_in;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = mp_obj_get_int(layer);
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(empty_node_class_set_layer_obj, empty_node_class_set_layer);


STATIC mp_obj_t empty_node_class_get_layer(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Getting object layer...");

    engine_node_base_t *node_base = self_in;
    return mp_obj_new_int(node_base->layer);
}
MP_DEFINE_CONST_FUN_OBJ_1(empty_node_class_get_layer_obj, empty_node_class_get_layer);


STATIC void empty_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing EmptyNode attr");

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&empty_node_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_layer:
                destination[0] = MP_OBJ_FROM_PTR(&empty_node_class_set_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_layer:
                destination[0] = MP_OBJ_FROM_PTR(&empty_node_class_get_layer_obj);
                destination[1] = self_in;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t empty_node_class_locals_dict_table[] = {

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
    .attr = empty_node_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&empty_node_class_locals_dict,
};