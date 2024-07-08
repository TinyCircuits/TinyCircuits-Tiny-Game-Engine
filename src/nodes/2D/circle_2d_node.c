#include "circle_2d_node.h"

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


void circle_2d_node_class_draw(mp_obj_t circle_node_base_obj, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Circle2DNode: Drawing");

    engine_node_base_t *circle_node_base = circle_node_base_obj;
    engine_circle_2d_node_class_obj_t *circle_2d_node = circle_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float circle_opacity = mp_obj_get_float(circle_2d_node->opacity);
    if(engine_math_compare_floats(circle_opacity, 0.0f)){
        return;
    }

    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    rectangle_class_obj_t *camera_viewport = camera->viewport;
    float camera_zoom = mp_obj_get_float(camera->zoom);

    float circle_scale =  mp_obj_get_float(circle_2d_node->scale);
    float circle_radius =  mp_obj_get_float(circle_2d_node->radius);
    bool circle_outlined = mp_obj_get_int(circle_2d_node->outline);

    color_class_obj_t *circle_color = circle_2d_node->color;

    float circle_resolved_hierarchy_x = 0.0f;
    float circle_resolved_hierarchy_y = 0.0f;
    float circle_resolved_hierarchy_rotation = 0.0f;
    bool circle_is_child_of_camera = false;
    node_base_get_child_absolute_xy(&circle_resolved_hierarchy_x, &circle_resolved_hierarchy_y, &circle_resolved_hierarchy_rotation, &circle_is_child_of_camera, circle_node_base);

    // Store the non-rotated x and y for a second
    float circle_rotated_x = circle_resolved_hierarchy_x;
    float circle_rotated_y = circle_resolved_hierarchy_y;
    float circle_rotation = circle_resolved_hierarchy_rotation;

    if(circle_is_child_of_camera == false){
        float camera_resolved_hierarchy_x = 0.0f;
        float camera_resolved_hierarchy_y = 0.0f;
        float camera_resolved_hierarchy_rotation = 0.0f;
        node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, NULL, camera_node);
        camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

        circle_rotated_x = (circle_rotated_x - camera_resolved_hierarchy_x) * camera_zoom;
        circle_rotated_y = (circle_rotated_y - camera_resolved_hierarchy_y) * camera_zoom;

        // Rotate rectangle origin about the camera
        engine_math_rotate_point(&circle_rotated_x, &circle_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

        circle_rotation += camera_resolved_hierarchy_rotation;
    }else{
        camera_zoom = 1.0f;
    }

    // The final circle radius to draw the circle at is a combination of
    // the set radius, times the set scale, times the set camera zoom.
    // Do this after determining if a child of a camera at any point
    // since in that case zoom shouldn't have an effect
    circle_radius = (circle_radius*circle_scale*camera_zoom);

    circle_rotated_x += camera_viewport->width/2;
    circle_rotated_y += camera_viewport->height/2;

    // Decide which shader to use per-pixel
    engine_shader_t *shader = NULL;
    if(circle_opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    if(circle_outlined == false){
        engine_draw_filled_circle(circle_color->value, floorf(circle_rotated_x), floorf(circle_rotated_y), circle_radius, circle_opacity, shader);
    }else{
        engine_draw_outline_circle(circle_color->value, floorf(circle_rotated_x), floorf(circle_rotated_y), circle_radius, circle_opacity, shader);
    }
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool circle_2d_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_circle_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_del_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_mark_destroy:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_mark_destroy_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_mark_destroy_all:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_mark_destroy_all_obj);
            destination[1] = self_node_base;
            return true;
        break;
        case MP_QSTR_mark_destroy_children:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_mark_destroy_children_obj);
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
        case MP_QSTR_get_child_count:
            destination[0] = MP_OBJ_FROM_PTR(&node_base_get_child_count_obj);
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
        case MP_QSTR_radius:
            destination[0] = self->radius;
            return true;
        break;
        case MP_QSTR_rotation:
            destination[0] = self->rotation;
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
        case MP_QSTR_scale:
            destination[0] = self->scale;
            return true;
        break;
        case MP_QSTR_outline:
            destination[0] = self->outline;
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool circle_2d_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_circle_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_radius:
            self->radius = destination[1];
            return true;
        break;
        case MP_QSTR_rotation:
            self->rotation = destination[1];
            return true;
        break;
        case MP_QSTR_color:
            self->color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_opacity:
            self->opacity = destination[1];
            return true;
        break;
        case MP_QSTR_scale:
            self->scale = destination[1];
            return true;
        break;
        case MP_QSTR_outline:
            self->outline = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t circle_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Circle2DNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = circle_2d_node_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = circle_2d_node_store_attr(node_base, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance && attr_handled == false){
        node_base_use_default_attr_handler(self_in, attribute, destination);
    }

    return mp_const_none;
}


/* --- doc ---
   NAME: Circle2DNode
   ID: Circle2DNode
   DESC: Simple node that draws a colored circle given a radius
   PARAM:   [type={ref_link:Vector2}]  [name=position]                                  [value={ref_link:Vector2}]
   PARAM:   [type=float]               [name=radius]                                    [value=any]
   PARAM:   [type={ref_link:Color}|int (RGB565)]  [name=color]                          [value=color]
   PARAM:   [type=float]               [name=opacity]                                   [value=0 ~ 1.0]
   PARAM:   [type=bool]                [name=outline]                                   [value=True or False]
   PARAM:   [type=float]               [name=rotation]                                  [value=any]
   PARAM:   [type=float]               [name=scale]                                     [value=any]
   ATTR:    [type=function]            [name={ref_link:add_child}]                      [value=function]
   ATTR:    [type=function]            [name={ref_link:get_child}]                      [value=function]
   ATTR:    [type=function]            [name={ref_link:get_child_count}]                [value=function]
   ATTR:    [type=function]            [name={ref_link:node_base_mark_destroy}]              [value=function]
   ATTR:    [type=function]            [name={ref_link:node_base_mark_destroy_all}]          [value=function]
   ATTR:    [type=function]            [name={ref_link:node_base_mark_destroy_children}]     [value=function]
   ATTR:    [type=function]            [name={ref_link:remove_child}]                   [value=function]
   ATTR:    [type=function]            [name={ref_link:set_layer}]                      [value=function]
   ATTR:    [type=function]            [name={ref_link:get_layer}]                      [value=function]
   ATTR:    [type=function]            [name={ref_link:tick}]                           [value=function]
   ATTR:    [type={ref_link:Vector2}]  [name=position]                                  [value={ref_link:Vector2}]
   ATTR:    [type=float]               [name=radius]                                    [value=any]
   ATTR:    [type=float]               [name=rotation]                                  [value=any]
   ATTR:    [type={ref_link:Color}|int (RGB565)]  [name=color]                          [value=color]
   ATTR:    [type=float]               [name=opacity]                                   [value=0 ~ 1.0]
   ATTR:    [type=float]               [name=scale]                                     [value=any]
   ATTR:    [type=bool]                [name=outline]                                   [value=True or False]
   OVRR:    [type=function]            [name={ref_link:tick}]                           [value=function]
*/
mp_obj_t circle_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Circle2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_radius,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_opacity,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_outline,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, radius, color, opacity, outline, rotation, scale};
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
    if(parsed_args[radius].u_obj == MP_OBJ_NULL) parsed_args[radius].u_obj = mp_obj_new_float(5.0f);
    if(parsed_args[color].u_obj == MP_OBJ_NULL) parsed_args[color].u_obj = MP_OBJ_NEW_SMALL_INT(0xffff);
    if(parsed_args[opacity].u_obj == MP_OBJ_NULL) parsed_args[opacity].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[outline].u_obj == MP_OBJ_NULL) parsed_args[outline].u_obj = mp_obj_new_bool(false);
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = mp_obj_new_float(1.0f);

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_circle_2d_node_class_type);
    node_base_init(node_base, &engine_circle_2d_node_class_type, NODE_TYPE_CIRCLE_2D);
    engine_circle_2d_node_class_obj_t *circle_2d_node = m_malloc(sizeof(engine_circle_2d_node_class_obj_t));
    node_base->node = circle_2d_node;
    node_base->attr_accessor = node_base;

    circle_2d_node->tick_cb  = mp_const_none;
    circle_2d_node->position = parsed_args[position].u_obj;
    circle_2d_node->radius = parsed_args[radius].u_obj;
    circle_2d_node->rotation = parsed_args[rotation].u_obj;
    circle_2d_node->color = engine_color_wrap(parsed_args[color].u_obj);
    circle_2d_node->opacity = parsed_args[opacity].u_obj;
    circle_2d_node->scale = parsed_args[scale].u_obj;
    circle_2d_node->outline = parsed_args[outline].u_obj;

    if(inherited == true){  // Inherited (use existing object)
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        node_base_set_attr_handler_default(node_instance);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];

        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            circle_2d_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            circle_2d_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, circle_2d_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t circle_2d_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(circle_2d_node_class_locals_dict, circle_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_circle_2d_node_class_type,
    MP_QSTR_Circle2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, circle_2d_node_class_new,
    attr, circle_2d_node_class_attr,
    locals_dict, &circle_2d_node_class_locals_dict
);
