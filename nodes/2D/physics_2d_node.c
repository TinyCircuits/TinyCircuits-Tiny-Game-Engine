#include "physics_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "draw/engine_display_draw.h"
#include "physics/engine_physics.h"


// Class required functions
STATIC void physics_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Physics2DNode");
}


STATIC mp_obj_t physics_2d_node_class_tick(mp_obj_t self_in){
    engine_node_base_t *node_base = self_in;
    engine_physics_2d_node_common_data_t *common_data = node_base->node_common_data;

    double x, y;

    engine_physics_get_body_xy(common_data->physac_body, &x, &y);

    ENGINE_WARNING_PRINTF("Physics2DNode: Tick function not overridden %0.3f %0.3f", x, y);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(physics_2d_node_class_tick_obj, physics_2d_node_class_tick);


mp_obj_t physics_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Physics2DNode");
    
    engine_physics_2d_node_common_data_t *common_data = malloc(sizeof(engine_physics_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_physics_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_PHYSICS_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    // Track the node base for this physics node so that it can
    // be looped over quickly in a linked list and have its
    // attributes copied back and forth quickly between
    // the engine and physics engine
    common_data->physics_list_node = engine_physics_track_node(node_base);

    ENGINE_INFO_PRINTF("Physics2DNode: Creating Box2d body...");
    engine_physics_create_rectangle_body(common_data);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_physics_2d_node_class_obj_t *physics_2d_node = m_malloc(sizeof(engine_physics_2d_node_class_obj_t));
        node_base->node = physics_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_tick_obj);

        physics_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        physics_2d_node->rotation = mp_obj_new_float(0.0f);
        physics_2d_node->velocity = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        physics_2d_node->acceleration = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        physics_2d_node->dynamic = mp_obj_new_bool(true);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&physics_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_velocity, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_acceleration, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_dynamic, mp_obj_new_bool(true));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to Physics2DNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC void physics_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Physics2DNode attr");

    engine_physics_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_velocity:
                destination[0] = self->velocity;
            break;
            case MP_QSTR_acceleration:
                destination[0] = self->acceleration;
            break;
            case MP_QSTR_dynamic:
                destination[0] = self->dynamic;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_velocity:
                self->velocity = destination[1];
            break;
            case MP_QSTR_acceleration:
                self->acceleration = destination[1];
            break;
            case MP_QSTR_dynamic:
                self->dynamic = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t physics_2d_node_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(physics_2d_node_class_locals_dict, physics_2d_node_class_locals_dict_table);

const mp_obj_type_t engine_physics_2d_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Physics2DNode,
    .print = physics_2d_node_class_print,
    .make_new = physics_2d_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = physics_2d_node_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&physics_2d_node_class_locals_dict,
};