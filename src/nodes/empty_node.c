#include "empty_node.h"

#include "node_types.h"
#include "node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"

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
    node_base->parent_node_base = NULL;
    node_base->location_in_parents_children = NULL;
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;
        node_base->node = NULL;
        node_base->attr_accessor = NULL;
        common_data->tick_cb = MP_OBJ_FROM_PTR(&empty_node_class_tick_obj);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&empty_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Too many arguments passed to EmptyNode constructor!"));
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC void empty_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing EmptyNode attr");

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_add_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_remove_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_layer:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_set_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_layer:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_node_base:
                destination[0] = self_in;
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
STATIC MP_DEFINE_CONST_DICT(empty_node_class_locals_dict, empty_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_empty_node_class_type,
    MP_QSTR_EmptyNode,
    MP_TYPE_FLAG_NONE,

    make_new, empty_node_class_new,
    print, empty_node_class_print,
    attr, empty_node_class_attr,
    locals_dict, &empty_node_class_locals_dict
);