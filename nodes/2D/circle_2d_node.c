#include "circle_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"

// Class required functions
STATIC void circle_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Circle2DNode");
}


STATIC mp_obj_t circle_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Circle2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(circle_2d_node_class_tick_obj, circle_2d_node_class_tick);


STATIC mp_obj_t circle_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Circle2DNode: Drawing");
    
    engine_node_base_t *circle_node_base = self_in;
    engine_node_base_t *camera_node_base = camera_node;

    int16_t circle_radius =  (uint16_t)(mp_obj_get_float(mp_load_attr(circle_node_base->attr_accessor, MP_QSTR_radius)));
    mp_int_t circle_color = mp_obj_get_int(mp_load_attr(circle_node_base->attr_accessor, MP_QSTR_color));
    int16_t circle_radius_sqr = circle_radius * circle_radius;

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);

    float circle_resolved_hierarchy_x = 0.0f;
    float circle_resolved_hierarchy_y = 0.0f;
    float circle_resolved_hierarchy_rotation = 0.0f;

    node_base_get_child_absolute_xy(&circle_resolved_hierarchy_x, &circle_resolved_hierarchy_y, &circle_resolved_hierarchy_rotation, self_in);

    // Store the non-rotated x and y for a second
    float circle_rotated_x = circle_resolved_hierarchy_x-(camera_position->x);
    float circle_rotated_y = circle_resolved_hierarchy_y-(camera_position->y);

    // Rotate rectangle origin about the camera
    engine_math_rotate_point(&circle_rotated_x, &circle_rotated_y, camera_viewport->width/2, camera_viewport->height/2, camera_rotation->z * DEG2RAD);

    // https://stackoverflow.com/a/59211338
    for(int x=-circle_radius; x<circle_radius; x++){
        int hh = (int)sqrt(circle_radius_sqr - x * x);
        int rx = circle_rotated_x + x;
        int ph = circle_rotated_y + hh;

        for(int y=circle_rotated_y-hh; y<ph; y++){
            engine_draw_pixel(circle_color, rx, y);
        }
    }
                                               
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(circle_2d_node_class_draw_obj, circle_2d_node_class_draw);


mp_obj_t circle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Circle2DNode");

    engine_circle_2d_node_common_data_t *common_data = malloc(sizeof(engine_circle_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_circle_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_CIRCLE_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_circle_2d_node_class_obj_t *circle_2d_node = m_malloc(sizeof(engine_circle_2d_node_class_obj_t));
        node_base->node = circle_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_draw_obj);

        circle_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        circle_2d_node->radius = mp_obj_new_float(5.0f);
        circle_2d_node->rotation = mp_obj_new_float(0.0f);
        circle_2d_node->color = mp_obj_new_int(0xffff);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&circle_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_radius, mp_obj_new_float(5.0f));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_color, mp_obj_new_int(0xffff));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to Circle2DNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC void circle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Circle2DNode attr");

    engine_circle_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_radius:
                destination[0] = self->radius;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_radius:
                self->radius = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t circle_2d_node_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(circle_2d_node_class_locals_dict, circle_2d_node_class_locals_dict_table);

const mp_obj_type_t engine_circle_2d_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Circle2DNode,
    .print = circle_2d_node_class_print,
    .make_new = circle_2d_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = circle_2d_node_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&circle_2d_node_class_locals_dict,
};
