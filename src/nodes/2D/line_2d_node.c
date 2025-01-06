#include "line_2d_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "nodes/3D/camera_node.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "draw/engine_shader.h"


void line_2d_node_class_draw(mp_obj_t line_node_base_obj, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Line2DNode: Drawing");

    engine_node_base_t *line_node_base = line_node_base_obj;
    engine_line_2d_node_class_obj_t *line_2d = line_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float line_opacity = mp_obj_get_float(line_2d->opacity);
    if(engine_math_compare_floats(line_opacity, 0.0f)){
        return;
    }

    engine_node_base_t *camera_node_base = camera_node;
    engine_camera_node_class_obj_t *camera = camera_node_base->node;

    vector2_class_obj_t *line_start = line_2d->start;
    vector2_class_obj_t *line_end = line_2d->end;
    float line_thickness = mp_obj_get_float(line_2d->thickness);
    color_class_obj_t *line_color = line_2d->color;
    bool line_outlined = mp_obj_get_int(line_2d->outline);

    // The line is drawn as a rectangle since we have a nice algorithm for doing that:
    float line_length = engine_math_distance_between(line_start->x.value, line_start->y.value, line_end->x.value, line_end->y.value);
    float line_rotation = engine_math_angle_between(line_start->x.value, line_start->y.value, line_end->x.value, line_end->y.value) + HALF_PI;

    // Grab camera
    rectangle_class_obj_t *camera_viewport = camera->viewport;
    float camera_zoom = mp_obj_get_float(camera->zoom);
    float camera_opacity = mp_obj_get_float(camera->opacity);

    // Get inherited properties
    engine_inheritable_2d_t inherited;
    node_base_inherit_2d(line_node_base, &inherited);
    inherited.rotation += line_rotation;

    if(inherited.is_camera_child == false){
        engine_camera_transform_2d(camera_node, &inherited.px, &inherited.py, &inherited.rotation);
    }else{
        camera_zoom = 1.0f;
    }

    inherited.px += camera_viewport->width/2;
    inherited.py += camera_viewport->height/2;

    line_opacity = inherited.opacity*camera_opacity;

    // Scale by camera
    line_thickness = line_thickness * inherited.sy * camera_zoom;
    line_length = line_length * inherited.sx * camera_zoom;

    // Stop line from disappearing when it gets too thin
    if(line_thickness < 1.0f){
        line_thickness = 1.0f;
    }

    // Decide which shader to use per-pixel
    engine_shader_t *shader = NULL;
    if(line_opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    if(line_outlined == false){
        engine_draw_rect(line_color->value,
                         floorf(inherited.px), floorf(inherited.py),
                         (int32_t)line_thickness, (int32_t)line_length,
                         1.0f, 1.0f,
                         inherited.rotation,
                         line_opacity,
                         shader);
    }else{
        float line_half_width = line_thickness/2.0f;
        float line_half_height = line_length/2.0f;

        // Calculate the coordinates of the 4 corners of the line, not rotated
        // NOTE: positive y is down
        float tlx = floorf(inherited.px - line_half_width);
        float tly = floorf(inherited.py - line_half_height);

        float trx = floorf(inherited.px + line_half_width);
        float try = floorf(inherited.py - line_half_height);

        float brx = floorf(inherited.px + line_half_width);
        float bry = floorf(inherited.py + line_half_height);

        float blx = floorf(inherited.px - line_half_width);
        float bly = floorf(inherited.py + line_half_height);

        // Rotate the points and then draw lines between them
        engine_math_rotate_point(&tlx, &tly, inherited.px, inherited.py, inherited.py);
        engine_math_rotate_point(&trx, &try, inherited.px, inherited.py, inherited.py);
        engine_math_rotate_point(&brx, &bry, inherited.px, inherited.py, inherited.py);
        engine_math_rotate_point(&blx, &bly, inherited.px, inherited.py, inherited.py);

        engine_draw_line(line_color->value, tlx, tly, trx, try, camera_node, line_opacity, shader);
        engine_draw_line(line_color->value, trx, try, brx, bry, camera_node, line_opacity, shader);
        engine_draw_line(line_color->value, brx, bry, blx, bly, camera_node, line_opacity, shader);
        engine_draw_line(line_color->value, blx, bly, tlx, tly, camera_node, line_opacity, shader);
    }
}


void line_2d_recalculate_midpoint(void *line_obj){
    engine_line_2d_node_class_obj_t *line = (engine_line_2d_node_class_obj_t*)line_obj;

    vector2_class_obj_t *start = line->start;
    vector2_class_obj_t *position = line->position;
    vector2_class_obj_t *end = line->end;

    float mx = 0.0f;
    float my = 0.0f;

    engine_math_2d_midpoint(start->x.value, start->y.value, end->x.value, end->y.value, &mx, &my);

    position->x.value = mx;
    position->y.value = my;
}


void line_2d_translate_endpoints(void *line_obj, float nx, float ny){
    engine_line_2d_node_class_obj_t *line = (engine_line_2d_node_class_obj_t*)line_obj;

    vector2_class_obj_t *start = line->start;
    vector2_class_obj_t *position = line->position;
    vector2_class_obj_t *end = line->end;

    float dx = nx - position->x.value;
    float dy = ny - position->y.value;

    start->x.value += dx;
    end->x.value += dx;

    start->y.value += dy;
    end->y.value += dy;
}


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool line_2d_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_line_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_start:
            destination[0] = self->start;
            return true;
        break;
        case MP_QSTR_end:
            destination[0] = self->end;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_thickness:
            destination[0] = self->thickness;
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
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool line_2d_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_line_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;
        case MP_QSTR_start:
            self->start = destination[1];
            line_2d_recalculate_midpoint(self);
            return true;
        break;
        case MP_QSTR_end:
            self->end = destination[1];
            line_2d_recalculate_midpoint(self);
            return true;
        break;
        case MP_QSTR_position:
            // Offset `start` and `end` based on new position
            line_2d_translate_endpoints(self, ((vector2_class_obj_t*)destination[1])->x.value, ((vector2_class_obj_t*)destination[1])->y.value);
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_thickness:
            self->thickness = destination[1];
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
        case MP_QSTR_outline:
            self->outline = destination[1];
            return true;
        break;
        default:
            return false; // Fail
    }
}


static mp_attr_fun_t line_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Line2DNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){node_base_load_attr, line_2d_load_attr},
                          (attr_handler_func[]){node_base_store_attr, line_2d_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: Line2DNode
    ID: Line2DNode
    DESC: Draws a line from `start` to `end`. Changing `position` (the midpoint of the line) automaticaly translates `end` and `start`
    PARAM:  [type={ref_link:Vector2}]               [name=start]                                        [value={ref_link:Vector2}]
    PARAM:  [type={ref_link:Vector2}]               [name=end]                                          [value={ref_link:Vector2}]
    PARAM:  [type=float]                            [name=thickness]                                    [value=any]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=color]                                        [value=color]
    PARAM:  [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]
    PARAM:  [type=bool]                             [name=outline]                                      [value=True or False]
    PARAM:  [type=int]                              [name=layer]                                        [value=0 ~ 127]
    PARAM:  [type=bool]                             [name=inherit_position]                             [value=True or False]
    PARAM:  [type=bool]                             [name=inherit_opacity]                              [value=True or False]
    PARAM:  [type=bool]                             [name=inherit_rotation]                             [value=True or False]
    PARAM:  [type=bool]                             [name=inherit_scale]                                [value=True or False]
    ATTR:   [type=function]                         [name={ref_link:add_child}]                         [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_child}]                         [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_child_count}]                   [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy}]            [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy_all}]        [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy_children}]   [value=function]
    ATTR:   [type=function]                         [name={ref_link:remove_child}]                      [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_parent}]                        [value=function]
    ATTR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
    ATTR:   [type={ref_link:Vector2}]               [name=start]                                        [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:Vector2}]               [name=end]                                          [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:Vector2}]               [name=position]                                     [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:Vector2}]               [name=global_position]                              [value={ref_link:Vector2} (read-only)]
    ATTR:   [type=float]                            [name=thickness]                                    [value=any]
    ATTR:   [type={ref_link:Color}|int (RGB565)]    [name=color]                                        [value=color]
    ATTR:   [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]
    ATTR:   [type=bool]                             [name=outline]                                      [value=True or False]
    ATTR:   [type=int]                              [name=layer]                                        [value=0 ~ 127]
    ATTR:   [type=bool]                             [name=inherit_position]                             [value=True or False]
    ATTR:   [type=bool]                             [name=inherit_opacity]                              [value=True or False]
    ATTR:   [type=bool]                             [name=inherit_rotation]                             [value=True or False]
    ATTR:   [type=bool]                             [name=inherit_scale]                                [value=True or False]
    OVRR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
*/
mp_obj_t line_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Line2DNode");

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,       MP_ARG_OBJ,  {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_start,             MP_ARG_OBJ,  {.u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(-5.0f)})} },
        { MP_QSTR_end,               MP_ARG_OBJ,  {.u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(5.0f)})} },
        { MP_QSTR_thickness,         MP_ARG_OBJ,  {.u_obj = mp_obj_new_float(1.0f)} },
        { MP_QSTR_color,             MP_ARG_OBJ,  {.u_obj = MP_OBJ_NEW_SMALL_INT(0xffff)} },
        { MP_QSTR_opacity,           MP_ARG_OBJ,  {.u_obj = mp_obj_new_float(1.0f)} },
        { MP_QSTR_outline,           MP_ARG_OBJ,  {.u_obj = mp_obj_new_bool(false)} },
        { MP_QSTR_layer,             MP_ARG_INT,  {.u_int = 0} },
        { MP_QSTR_inherit_position,  MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_inherit_opacity,   MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_inherit_rotation,  MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_inherit_scale,     MP_ARG_BOOL, {.u_bool = true} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, start, end, thickness, color, opacity, outline, layer, inherit_position, inherit_opacity, inherit_rotation, inherit_scale};
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

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_line_2d_node_class_type);
    node_base_init(node_base, &engine_line_2d_node_class_type, NODE_TYPE_LINE_2D, parsed_args[layer].u_int);

    engine_line_2d_node_class_obj_t *line_2d_node = m_malloc(sizeof(engine_line_2d_node_class_obj_t));
    node_base->node = line_2d_node;
    node_base->attr_accessor = node_base;

    line_2d_node->tick_cb = mp_const_none;

    line_2d_node->start = parsed_args[start].u_obj;
    line_2d_node->end = parsed_args[end].u_obj;
    line_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    line_2d_node->thickness = parsed_args[thickness].u_obj;
    line_2d_node->color = engine_color_wrap(parsed_args[color].u_obj);
    line_2d_node->opacity = parsed_args[opacity].u_obj;
    line_2d_node->outline = parsed_args[outline].u_obj;
    node_base_set_inherit_position(node_base, parsed_args[inherit_position].u_bool);
    node_base_set_inherit_opacity(node_base, parsed_args[inherit_opacity].u_bool);
    node_base_set_inherit_rotation(node_base, parsed_args[inherit_rotation].u_bool);
    node_base_set_inherit_scale(node_base, parsed_args[inherit_scale].u_bool);

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        node_base_set_attr_handler_default(node_instance);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            line_2d_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            line_2d_node->tick_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, line_2d_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    // Calculate midpoint/position based on endpoints
    // (only positions that can be set in the constructor)
    line_2d_recalculate_midpoint(line_2d_node);

    // When any part of the any of these Vector2s change, make sure to
    // recalculate other components of the line
    vector2_class_obj_t *line_start = line_2d_node->start;
    vector2_class_obj_t *line_position = line_2d_node->position;
    vector2_class_obj_t *line_end = line_2d_node->end;

    line_start->on_changed = &line_2d_recalculate_midpoint;
    line_start->on_change_user_ptr = line_2d_node;

    line_position->on_changing = &line_2d_translate_endpoints;
    line_position->on_change_user_ptr = line_2d_node;

    line_end->on_changed = &line_2d_recalculate_midpoint;
    line_end->on_change_user_ptr = line_2d_node;

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t line_2d_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(line_2d_node_class_locals_dict, line_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_line_2d_node_class_type,
    MP_QSTR_Line2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, line_2d_node_class_new,
    attr, line_2d_node_class_attr,
    locals_dict, &line_2d_node_class_locals_dict
);