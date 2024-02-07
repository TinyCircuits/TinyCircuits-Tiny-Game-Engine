#include "rectangle_2d_node.h"

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
    
    // Decode and store properties about the rectangle and camera nodes
    engine_node_base_t *rectangle_node_base = self_in;
    engine_node_base_t *camera_node_base = camera_node;

    vector2_class_obj_t *rectangle_scale =  mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_scale);
    uint16_t rectangle_width = mp_obj_get_int(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_width));
    uint16_t rectangle_height = mp_obj_get_int(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_height));
    uint16_t rectangle_color = mp_obj_get_int(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_color));
    bool rectangle_outlined = mp_obj_get_int(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_outline));

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
    float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

    rectangle_width = rectangle_width*camera_zoom;
    rectangle_height = rectangle_height*camera_zoom;

    float camera_resolved_hierarchy_x = 0.0f;
    float camera_resolved_hierarchy_y = 0.0f;
    float camera_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, camera_node);
    camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

    float rectangle_resolved_hierarchy_x = 0.0f;
    float rectangle_resolved_hierarchy_y = 0.0f;
    float rectangle_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&rectangle_resolved_hierarchy_x, &rectangle_resolved_hierarchy_y, &rectangle_resolved_hierarchy_rotation, self_in);

    // Store the non-rotated x and y for a second
    float rectangle_rotated_x = rectangle_resolved_hierarchy_x-camera_resolved_hierarchy_x;
    float rectangle_rotated_y = rectangle_resolved_hierarchy_y-camera_resolved_hierarchy_y;

    // Scale transformation due to camera zoom
    engine_math_scale_point(&rectangle_rotated_x, &rectangle_rotated_y, camera_position->x, camera_position->y, camera_zoom);

    // Rotate rectangle origin about the camera
    engine_math_rotate_point(&rectangle_rotated_x, &rectangle_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

    rectangle_rotated_x += camera_viewport->width/2;
    rectangle_rotated_y += camera_viewport->height/2;

    if(rectangle_outlined == false){
        engine_draw_fillrect_scale_rotate_viewport(rectangle_color,
                                                (int32_t)rectangle_rotated_x,
                                                (int32_t)rectangle_rotated_y,
                                                rectangle_width, 
                                                rectangle_height,
                                                (int32_t)(rectangle_scale->x*65536 + 0.5),
                                                (int32_t)(rectangle_scale->y*65536 + 0.5),
                                                (int16_t)(((rectangle_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation))*1024 / (float)(2*PI)),
                                                (int32_t)camera_viewport->x,
                                                (int32_t)camera_viewport->y,
                                                (int32_t)camera_viewport->width,
                                                (int32_t)camera_viewport->height);
    }else{
        float rectangle_half_width = rectangle_width/2;
        float rectangle_half_height = rectangle_height/2;

        // Calculate the coordinates of the 4 corners of the rectangle, not rotated
        // NOTE: positive y is down
        float tlx = rectangle_rotated_x - rectangle_half_width;
        float tly = rectangle_rotated_y - rectangle_half_height;

        float trx = rectangle_rotated_x + rectangle_half_width;
        float try = rectangle_rotated_y - rectangle_half_height;

        float brx = rectangle_rotated_x + rectangle_half_width;
        float bry = rectangle_rotated_y + rectangle_half_height;

        float blx = rectangle_rotated_x - rectangle_half_width;
        float bly = rectangle_rotated_y + rectangle_half_height;

        // Rotate the points and then draw lines between them
        float angle = rectangle_resolved_hierarchy_rotation + camera_resolved_hierarchy_rotation;
        engine_math_rotate_point(&tlx, &tly, rectangle_rotated_x, rectangle_rotated_y, angle);
        engine_math_rotate_point(&trx, &try, rectangle_rotated_x, rectangle_rotated_y, angle);
        engine_math_rotate_point(&brx, &bry, rectangle_rotated_x, rectangle_rotated_y, angle);
        engine_math_rotate_point(&blx, &bly, rectangle_rotated_x, rectangle_rotated_y, angle);

        engine_draw_line(rectangle_color, tlx, tly, trx, try, camera_node);
        engine_draw_line(rectangle_color, trx, try, brx, bry, camera_node);
        engine_draw_line(rectangle_color, brx, bry, blx, bly, camera_node);
        engine_draw_line(rectangle_color, blx, bly, tlx, tly, camera_node);
    }
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
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_draw_obj);

        rectangle_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        rectangle_2d_node->scale = vector2_class_new(&vector2_class_type, 2, 0, default_scale_parameters);
        rectangle_2d_node->width = mp_obj_new_int(15);
        rectangle_2d_node->height = mp_obj_new_int(5);
        rectangle_2d_node->color = mp_obj_new_int(0xffff);
        rectangle_2d_node->rotation = mp_obj_new_float(0.0f);
        rectangle_2d_node->outline = mp_obj_new_bool(false);
    }else if(n_args == 1){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = args[0];
        node_base->attr_accessor = node_base->node;

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
        mp_store_attr(node_base->node, MP_QSTR_rotation, mp_obj_new_float(0.0f));
        mp_store_attr(node_base->node, MP_QSTR_outline, mp_obj_new_bool(false));
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Too many arguments passed to Rectangle2DNode constructor!"));
    }

    return MP_OBJ_FROM_PTR(node_base);
}


/*  --- doc ---
    NAME: Rectangle2DNode
    DESC: Simple 2D rectangle node
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector3}]         [name=position]                   [value={ref_link:Vector3}]
    ATTR:   [type=float]                      [name=fps]                        [value=any]
    ATTR:   [type=float]                      [name=rotation]                   [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=float]                      [name=width]                      [value=any]
    ATTR:   [type=float]                      [name=height]                     [value=any]
    ATTR:   [type=int]                        [name=color]                      [value=any 16-bit RGB565 integer]
    ATTR:   [type=bool]                       [name=outline]                    [value=True or False]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    OVRR:   [type=function]                   [name={ref_link:draw}]            [value=function]
*/
STATIC void rectangle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Rectangle2DNode attr");

    engine_rectangle_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_get_child:
                destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
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
            case MP_QSTR_outline:
                destination[0] = self->outline;
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
            case MP_QSTR_outline:
                self->outline = destination[1];
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
STATIC MP_DEFINE_CONST_DICT(rectangle_2d_node_class_locals_dict, rectangle_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_rectangle_2d_node_class_type,
    MP_QSTR_Rectangle2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, rectangle_2d_node_class_new,
    print, rectangle_2d_node_class_print,
    attr, rectangle_2d_node_class_attr,
    locals_dict, &rectangle_2d_node_class_locals_dict
);