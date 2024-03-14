#include "rectangle_2d_node.h"

#include "nodes/node_types.h"
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


void rectangle_2d_node_class_draw(engine_node_base_t *rectangle_node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Rectangle2DNode: Drawing");
    
    // Decode and store properties about the rectangle and camera nodes
    engine_node_base_t *camera_node_base = camera_node;

    vector2_class_obj_t *rectangle_scale =  mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_scale);
    uint16_t rectangle_width = mp_obj_get_float(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_width));
    uint16_t rectangle_height = mp_obj_get_float(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_height));
    uint16_t rectangle_color = mp_obj_get_int(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_color));
    bool rectangle_outlined = mp_obj_get_int(mp_load_attr(rectangle_node_base->attr_accessor, MP_QSTR_outline));

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
    float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

    float camera_resolved_hierarchy_x = 0.0f;
    float camera_resolved_hierarchy_y = 0.0f;
    float camera_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, NULL, camera_node);
    camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

    float rectangle_resolved_hierarchy_x = 0.0f;
    float rectangle_resolved_hierarchy_y = 0.0f;
    float rectangle_resolved_hierarchy_rotation = 0.0f;
    bool rectangle_is_child_of_camera = false;
    node_base_get_child_absolute_xy(&rectangle_resolved_hierarchy_x, &rectangle_resolved_hierarchy_y, &rectangle_resolved_hierarchy_rotation, &rectangle_is_child_of_camera, rectangle_node_base);

    // Store the non-rotated x and y for a second
    float rectangle_rotated_x = rectangle_resolved_hierarchy_x-camera_resolved_hierarchy_x;
    float rectangle_rotated_y = rectangle_resolved_hierarchy_y-camera_resolved_hierarchy_y;

    // Scale transformation due to camera zoom
    if(rectangle_is_child_of_camera == false){
        engine_math_scale_point(&rectangle_rotated_x, &rectangle_rotated_y, camera_position->x, camera_position->y, camera_zoom);
    }else{
        camera_zoom = 1.0f;
    }

    rectangle_width = rectangle_width*camera_zoom;
    rectangle_height = rectangle_height*camera_zoom;

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
}


/*  --- doc ---
    NAME: Rectangle2DNode
    DESC: Simple 2D rectangle node
    PARAM:  [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    PARAM:  [type=float]                      [name=width]                      [value=any]
    PARAM:  [type=float]                      [name=height]                     [value=any]
    PARAM:  [type=int]                        [name=color]                      [value=any 16-bit RGB565 integer]
    PARAM:  [type=bool]                       [name=outline]                    [value=True or False]
    PARAM:  [type=float]                      [name=rotation]                   [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    ATTR:   [type=float]                      [name=width]                      [value=any]
    ATTR:   [type=float]                      [name=height]                     [value=any]
    ATTR:   [type=int]                        [name=color]                      [value=any 16-bit RGB565 integer]
    ATTR:   [type=bool]                       [name=outline]                    [value=True or False]
    ATTR:   [type=float]                      [name=rotation]                   [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]

    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
*/
mp_obj_t rectangle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Rectangle2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_width,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_height,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_outline,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, width, height, color, outline, rotation, scale};
    bool inherited = false;

    // If there is one positional argument and it isn't the first 
    // expected argument (as is expected when using positional
    // arguments) then define which way to parse the arguments
    if(n_args >= 1 && mp_obj_get_type(args[0]) != &vector2_class_type){
        // Using positional arguments but the type of the first one isn't
        // as expected. Must be the child class
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);
        inherited = true;
    }else{
        // Whether we're using positional arguments or not, prase them this
        // way. It's a requirement that the child class be passed using position.
        // Adjust what and where the arguments are parsed, since not inherited based
        // on the first argument
        mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args)-1, allowed_args+1, parsed_args+1);
        inherited = false;
    }

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    if(parsed_args[width].u_obj == MP_OBJ_NULL) parsed_args[width].u_obj = mp_obj_new_float(15.0f);
    if(parsed_args[height].u_obj == MP_OBJ_NULL) parsed_args[height].u_obj = mp_obj_new_float(5.0f);
    if(parsed_args[color].u_obj == MP_OBJ_NULL) parsed_args[color].u_obj = mp_obj_new_int(0xffff);
    if(parsed_args[outline].u_obj == MP_OBJ_NULL) parsed_args[outline].u_obj = mp_obj_new_bool(false);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

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

    if(inherited == false){        // Non-inherited (create a new object)
        engine_rectangle_2d_node_class_obj_t *rectangle_2d_node = m_malloc(sizeof(engine_rectangle_2d_node_class_obj_t));
        node_base->node = rectangle_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_tick_obj);

        rectangle_2d_node->position = parsed_args[position].u_obj;
        rectangle_2d_node->width = parsed_args[width].u_obj;
        rectangle_2d_node->height = parsed_args[height].u_obj;
        rectangle_2d_node->color = parsed_args[color].u_obj;
        rectangle_2d_node->outline = parsed_args[outline].u_obj;
        rectangle_2d_node->rotation = parsed_args[rotation].u_obj;
        rectangle_2d_node->scale = parsed_args[scale].u_obj;
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->node = parsed_args[child_class].u_obj;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&rectangle_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_width, parsed_args[width].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_height, parsed_args[height].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_color, parsed_args[color].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_outline, parsed_args[outline].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_scale, parsed_args[scale].u_obj);
    }

    return MP_OBJ_FROM_PTR(node_base);
}


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
            case MP_QSTR_width:
                destination[0] = self->width;
            break;
            case MP_QSTR_height:
                destination[0] = self->height;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            case MP_QSTR_outline:
                destination[0] = self->outline;
            break;
            case MP_QSTR_rotation:
                destination[0] = self->rotation;
            break;
            case MP_QSTR_scale:
                destination[0] = self->scale;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
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
            case MP_QSTR_outline:
                self->outline = destination[1];
            break;
            case MP_QSTR_rotation:
                self->rotation = destination[1];
            break;
            case MP_QSTR_scale:
                self->scale = destination[1];
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