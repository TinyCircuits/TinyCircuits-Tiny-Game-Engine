#include "rectangle_2d_node.h"

#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"

// Class required functions
STATIC void rectangle_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Rectangle2DNode");
}


STATIC mp_obj_t rectangle_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Rectangle2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_2d_node_class_tick_obj, rectangle_2d_node_class_tick);


STATIC mp_obj_t rectangle_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Rectangle2DNode: Drawing");

    vector3_class_obj_t *camera_position = mp_load_attr(camera_node, MP_QSTR_position);
    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node, MP_QSTR_rotation);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node, MP_QSTR_viewport);

    int32_t vx = (int32_t)mp_obj_get_float(camera_viewport->x);
    int32_t vy = (int32_t)mp_obj_get_float(camera_viewport->y);
    uint8_t vw = (uint8_t)mp_obj_get_float(camera_viewport->width);
    uint8_t vh = (uint8_t)mp_obj_get_float(camera_viewport->height);
    const float cam_theta = mp_obj_get_float(camera_rotation->z);

    int32_t cx = (int32_t)mp_obj_get_float(camera_position->x);
    int32_t cy = (int32_t)mp_obj_get_float(camera_position->y);

    vector2_class_obj_t *position = mp_load_attr(self_in, MP_QSTR_position);
    mp_int_t width = mp_obj_get_int(mp_load_attr(self_in, MP_QSTR_width));
    mp_int_t height = mp_obj_get_int(mp_load_attr(self_in, MP_QSTR_height));
    mp_int_t color = mp_obj_get_int(mp_load_attr(self_in, MP_QSTR_color));

    int32_t px = (int32_t)mp_obj_get_float(position->x);
    int32_t py = (int32_t)mp_obj_get_float(position->y);

    vector2_class_obj_t *scale = mp_load_attr(self_in, MP_QSTR_scale);
    mp_int_t xsc = (int32_t)(mp_obj_get_float(scale->x)*65536 + 0.5);
    mp_int_t ysc = (int32_t)(mp_obj_get_float(scale->y)*65536 + 0.5);

    mp_float_t theta = mp_obj_get_float(mp_load_attr(self_in, MP_QSTR_rotation));

    // Rotation not implemented yet so this is simple!
    // for(mp_int_t y=0; y<height; y++){
    //     for(mp_int_t x=0; x<width; x++){
    //         engine_draw_pixel_viewport(color, px+x, py+y, vx, vy, vw, vh, cx, cy);
    //     }
    // }
    //ENGINE_INFO_PRINTF("Rendering at %i, %i\n\r", vx, vy);
    engine_draw_fillrect_scale_rotate_viewport(color, px+vx, py+vy, width, height, xsc, ysc, (int16_t)((theta-cam_theta)*1024 / (2*M_PI)), vx, vy, vw, vh);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rectangle_2d_node_class_draw_obj, rectangle_2d_node_class_draw);


mp_obj_t rectangle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Rectangle2DNode");

    engine_rectangle_2d_node_common_data_t *common_data = malloc(sizeof(engine_rectangle_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_rectangle_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_RECTANGLE_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    mp_obj_t default_scale_parameters[2];
    default_scale_parameters[0] = mp_obj_new_float(1.0f);
    default_scale_parameters[1] = mp_obj_new_float(1.0f);

    if(n_args == 0){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_rectangle_2d_node_class_obj_t *rectangle_2d_node = m_malloc(sizeof(engine_rectangle_2d_node_class_obj_t));
        node_base->node = rectangle_2d_node;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_draw_obj);

        rectangle_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        rectangle_2d_node->scale = vector2_class_new(&vector2_class_type, 2, 0, default_scale_parameters);
        rectangle_2d_node->width = mp_obj_new_int(15);
        rectangle_2d_node->height = mp_obj_new_int(5);
        rectangle_2d_node->color = mp_obj_new_int(0xffff);
        rectangle_2d_node->rotation = mp_obj_new_float(0.f);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_scale, vector2_class_new(&vector2_class_type, 2, 0, default_scale_parameters));
        mp_store_attr(node_base->node, MP_QSTR_width, mp_obj_new_int(15));
        mp_store_attr(node_base->node, MP_QSTR_height, mp_obj_new_int(5));
        mp_store_attr(node_base->node, MP_QSTR_color, mp_obj_new_int(0xffff));
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.f));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Too many arguments passed to Rectangle2DNode constructor!");
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class methods
STATIC mp_obj_t rectangle_2d_node_class_del(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Rectangle2DNode: Deleted (garbage collected, removing self from active engine objects)");

    engine_node_base_t *node_base = self_in;
    free(node_base->node_common_data);
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_2d_node_class_del_obj, rectangle_2d_node_class_del);


STATIC mp_obj_t rectangle_2d_node_class_set_layer(mp_obj_t self_in, mp_obj_t layer){
    ENGINE_INFO_PRINTF("Setting object to layer %d", mp_obj_get_int(layer));

    engine_node_base_t *node_base = self_in;
    engine_remove_object_from_layer(node_base->object_list_node, node_base->layer);
    node_base->layer = mp_obj_get_int(layer);
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(rectangle_2d_node_class_set_layer_obj, rectangle_2d_node_class_set_layer);


STATIC mp_obj_t rectangle_2d_node_class_get_layer(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Getting object layer...");

    engine_node_base_t *node_base = self_in;
    return mp_obj_new_int(node_base->layer);
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_2d_node_class_get_layer_obj, rectangle_2d_node_class_get_layer);


STATIC void rectangle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Rectangle2DNode attr");

    engine_rectangle_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_layer:
                destination[0] = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_set_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_layer:
                destination[0] = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_get_layer_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_scale:
                destination[0] = self->scale;
            break;
            case MP_QSTR_width:
                destination[0] = self->width;
            break;
            case MP_QSTR_height:
                destination[0] = self->height;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_scale:
                self->scale = destination[1];
            break;
            case MP_QSTR_width:
                self->width = destination[1];
            break;
            case MP_QSTR_height:
                self->height = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_2d_node_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(rectangle_2d_node_class_locals_dict, rectangle_2d_node_class_locals_dict_table);

const mp_obj_type_t engine_rectangle_2d_node_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Rectangle2DNode,
    .print = rectangle_2d_node_class_print,
    .make_new = rectangle_2d_node_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = rectangle_2d_node_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&rectangle_2d_node_class_locals_dict,
};
