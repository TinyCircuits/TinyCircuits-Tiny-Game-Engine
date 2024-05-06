#include "rectangle_2d_node.h"

#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "nodes/3D/camera_node.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "draw/engine_shader.h"


void rectangle_2d_node_class_draw(engine_node_base_t *rectangle_node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Rectangle2DNode: Drawing");
    
    // Decode and store properties about the rectangle and camera nodes
    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    engine_rectangle_2d_node_class_obj_t *rectangle_2d_node = rectangle_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float rectangle_opacity = mp_obj_get_float(rectangle_2d_node->opacity);
    if(engine_math_compare_floats(rectangle_opacity, 0.0f)){
        return;
    }

    vector2_class_obj_t *rectangle_scale =  rectangle_2d_node->scale;
    uint16_t rectangle_width = mp_obj_get_float(rectangle_2d_node->width);
    uint16_t rectangle_height = mp_obj_get_float(rectangle_2d_node->height);
    color_class_obj_t *rectangle_color = rectangle_2d_node->color;
    bool rectangle_outlined = mp_obj_get_int(rectangle_2d_node->outline);

    vector3_class_obj_t *camera_rotation = camera->rotation;
    vector3_class_obj_t *camera_position = camera->position;
    rectangle_class_obj_t *camera_viewport = camera->viewport;
    float camera_zoom = mp_obj_get_float(camera->zoom);

    float rectangle_resolved_hierarchy_x = 0.0f;
    float rectangle_resolved_hierarchy_y = 0.0f;
    float rectangle_resolved_hierarchy_rotation = 0.0f;
    bool rectangle_is_child_of_camera = false;
    node_base_get_child_absolute_xy(&rectangle_resolved_hierarchy_x, &rectangle_resolved_hierarchy_y, &rectangle_resolved_hierarchy_rotation, &rectangle_is_child_of_camera, rectangle_node_base);

    // Store the non-rotated x and y for a second
    float rectangle_rotated_x = rectangle_resolved_hierarchy_x;
    float rectangle_rotated_y = rectangle_resolved_hierarchy_y;
    float rectangle_rotation = rectangle_resolved_hierarchy_rotation;

    if(rectangle_is_child_of_camera == false){
        float camera_resolved_hierarchy_x = 0.0f;
        float camera_resolved_hierarchy_y = 0.0f;
        float camera_resolved_hierarchy_rotation = 0.0f;
        node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, NULL, camera_node);
        camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

        rectangle_rotated_x -= camera_resolved_hierarchy_x;
        rectangle_rotated_y -= camera_resolved_hierarchy_y;

        // Scale transformation due to camera zoom
        engine_math_scale_point(&rectangle_rotated_x, &rectangle_rotated_y, camera_position->x.value, camera_position->y.value, camera_zoom);

        // Rotate rectangle origin about the camera
        engine_math_rotate_point(&rectangle_rotated_x, &rectangle_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

        rectangle_rotation += camera_resolved_hierarchy_rotation;
    }else{
        camera_zoom = 1.0f;
    }

    rectangle_width = rectangle_width*camera_zoom;
    rectangle_height = rectangle_height*camera_zoom;

    rectangle_rotated_x += camera_viewport->width/2;
    rectangle_rotated_y += camera_viewport->height/2;

    // Decide which shader to use per-pixel
    engine_shader_t *shader = &empty_shader;
    if(rectangle_opacity < 1.0f){
        shader = &opacity_shader;
    }

    if(rectangle_outlined == false){
        engine_draw_rect(rectangle_color->value.val,
                         floorf(rectangle_rotated_x), floorf(rectangle_rotated_y),
                         rectangle_width, rectangle_height,
                         rectangle_scale->x.value*camera_zoom, rectangle_scale->y.value*camera_zoom,
                         -rectangle_rotation,
                         rectangle_opacity,
                         shader);
    }else{
        float rectangle_half_width = rectangle_width/2;
        float rectangle_half_height = rectangle_height/2;

        // Calculate the coordinates of the 4 corners of the rectangle, not rotated
        // NOTE: positive y is down
        float tlx = floorf(rectangle_rotated_x - rectangle_half_width);
        float tly = floorf(rectangle_rotated_y - rectangle_half_height);

        float trx = floorf(rectangle_rotated_x + rectangle_half_width);
        float try = floorf(rectangle_rotated_y - rectangle_half_height);

        float brx = floorf(rectangle_rotated_x + rectangle_half_width);
        float bry = floorf(rectangle_rotated_y + rectangle_half_height);

        float blx = floorf(rectangle_rotated_x - rectangle_half_width);
        float bly = floorf(rectangle_rotated_y + rectangle_half_height);

        // Rotate the points and then draw lines between them
        engine_math_rotate_point(&tlx, &tly, rectangle_rotated_x, rectangle_rotated_y, rectangle_rotation);
        engine_math_rotate_point(&trx, &try, rectangle_rotated_x, rectangle_rotated_y, rectangle_rotation);
        engine_math_rotate_point(&brx, &bry, rectangle_rotated_x, rectangle_rotated_y, rectangle_rotation);
        engine_math_rotate_point(&blx, &bly, rectangle_rotated_x, rectangle_rotated_y, rectangle_rotation);

        engine_draw_line(rectangle_color->value.val, tlx, tly, trx, try, camera_node, rectangle_opacity, shader);
        engine_draw_line(rectangle_color->value.val, trx, try, brx, bry, camera_node, rectangle_opacity, shader);
        engine_draw_line(rectangle_color->value.val, brx, bry, blx, bly, camera_node, rectangle_opacity, shader);
        engine_draw_line(rectangle_color->value.val, blx, bly, tlx, tly, camera_node, rectangle_opacity, shader);
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool rectangle_2d_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_rectangle_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_add_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_add_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_remove_child:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_remove_child_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_set_layer:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_set_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_get_layer:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_tick:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_layer_obj);
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_node_base:
            destination[0] = self_node_base;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_width:
            destination[0] = self->width;
            return true;
        break;
        case MP_QSTR_height:
            destination[0] = self->height;
            return true;
        break;
        case MP_QSTR_color:
            destination[0] = self->color;
            return true;
        break;
        case MP_QSTR_opacity:
            destination[0] = self->opacity;
            return true;
        break;
        case MP_QSTR_outline:
            destination[0] = self->outline;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
            return true;
        break;
        case MP_QSTR_scale:
            destination[0] = self->scale;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool rectangle_2d_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_rectangle_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_width:
            self->width = destination[1];
            return true;
        break;
        case MP_QSTR_height:
            self->height = destination[1];
            return true;
        break;
        case MP_QSTR_color:
            self->color = destination[1];
            return true;
        break;
        case MP_QSTR_opacity:
            self->opacity = destination[1];
            return true;
        break;
        case MP_QSTR_outline:
            self->outline = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_scale:
            self->scale = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t rectangle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Rectangle2DNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = rectangle_2d_node_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = rectangle_2d_node_store_attr(node_base, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance && attr_handled == false){
        default_instance_attr_func(self_in, attribute, destination);
    }

    return mp_const_none;
}


/*  --- doc ---
    NAME: Rectangle2DNode
    ID: Rectangle2DNode
    DESC: Simple 2D rectangle node
    PARAM:  [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    PARAM:  [type=float]                      [name=width]                      [value=any]
    PARAM:  [type=float]                      [name=height]                     [value=any]
    PARAM:  [type=int]                        [name=color]                      [value=any 16-bit RGB565 integer]
    PARAM:  [type=float]                      [name=opacity]                    [value=0 ~ 1.0] 
    PARAM:  [type=bool]                       [name=outline]                    [value=True or False]
    PARAM:  [type=float]                      [name=rotation]                   [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    ATTR:   [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    ATTR:   [type=float]                      [name=width]                      [value=any]
    ATTR:   [type=float]                      [name=height]                     [value=any]
    ATTR:   [type=int]                        [name=color]                      [value=any 16-bit RGB565 integer]
    ATTR:   [type=float]                      [name=opacity]                    [value=0 ~ 1.0] 
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
        { MP_QSTR_opacity,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_outline,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, width, height, color, opacity, outline, rotation, scale};
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
    if(parsed_args[width].u_obj == MP_OBJ_NULL) parsed_args[width].u_obj = mp_obj_new_float(10.0f);
    if(parsed_args[height].u_obj == MP_OBJ_NULL) parsed_args[height].u_obj = mp_obj_new_float(10.0f);
    if(parsed_args[color].u_obj == MP_OBJ_NULL) parsed_args[color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xffff)});
    if(parsed_args[opacity].u_obj == MP_OBJ_NULL) parsed_args[opacity].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[outline].u_obj == MP_OBJ_NULL) parsed_args[outline].u_obj = mp_obj_new_bool(false);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, &engine_rectangle_2d_node_class_type, NODE_TYPE_RECTANGLE_2D);
    engine_rectangle_2d_node_class_obj_t *rectangle_2d_node = m_malloc(sizeof(engine_rectangle_2d_node_class_obj_t));
    node_base->node = rectangle_2d_node;
    node_base->attr_accessor = node_base;

    rectangle_2d_node->tick_cb = mp_const_none;
    rectangle_2d_node->position = parsed_args[position].u_obj;
    rectangle_2d_node->width = parsed_args[width].u_obj;
    rectangle_2d_node->height = parsed_args[height].u_obj;
    rectangle_2d_node->color = parsed_args[color].u_obj;
    rectangle_2d_node->opacity = parsed_args[opacity].u_obj;
    rectangle_2d_node->outline = parsed_args[outline].u_obj;
    rectangle_2d_node->rotation = parsed_args[rotation].u_obj;
    rectangle_2d_node->scale = parsed_args[scale].u_obj;

    if(inherited == true){  // Inherited (use existing object)
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            rectangle_2d_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            rectangle_2d_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr);
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, rectangle_2d_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
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
    attr, rectangle_2d_node_class_attr,
    locals_dict, &rectangle_2d_node_class_locals_dict
);