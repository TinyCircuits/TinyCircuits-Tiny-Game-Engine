#include "line_2d_node.h"

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
STATIC void line_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Line2DNode");
}


STATIC mp_obj_t line_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Line2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(line_2d_node_class_tick_obj, line_2d_node_class_tick);


STATIC mp_obj_t line_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Line2DNode: Drawing");
    
    // Decode and store properties about the line and camera nodes
    engine_node_base_t *line_node_base = self_in;
    engine_node_base_t *camera_node_base = camera_node;

    vector2_class_obj_t *line_start = mp_load_attr(line_node_base->attr_accessor, MP_QSTR_start);
    vector2_class_obj_t *line_end = mp_load_attr(line_node_base->attr_accessor, MP_QSTR_end);
    float line_thickness = mp_obj_get_float(mp_load_attr(line_node_base->attr_accessor, MP_QSTR_thickness));
    uint16_t line_color = mp_obj_get_float(mp_load_attr(line_node_base->attr_accessor, MP_QSTR_color));
    bool line_outlined = mp_obj_get_int(mp_load_attr(line_node_base->attr_accessor, MP_QSTR_outline));

    float angle_between = engine_math_angle_between(line_start->x, line_start->y, line_end->x, line_end->y);

    // Handling line transforms would be weird, not implemented yet
    // node_base_get_child_absolute_xy

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(line_2d_node_class_draw_obj, line_2d_node_class_draw);


STATIC void line_2d_node_class_set(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Line2DNode: Accessing attr on inherited instance class");

    default_instance_attr_func(self_in, attribute, destination);

    // if(destination[0] == MP_OBJ_NULL){  // Load
    //     // Call this after the if statement we're in
    //     default_instance_attr_func(self_in, attribute, destination);
    // }else{                              // Store
    //     // Call this after the if statement we're in                     
    //     default_instance_attr_func(self_in, attribute, destination);
    //     switch(attribute){
    //         case MP_QSTR_text:
    //         {
    //             text_2d_node_class_calculate_dimensions(self_in, true);
    //         }
    //         break;
    //         case MP_QSTR_width:
    //         {
    //             mp_raise_msg(&mp_type_AttributeError, MP_ERROR_TEXT("Text2DNode: ERROR: 'width' is read-only, it is not allowed to be set!"));
    //         }
    //         break;
    //         case MP_QSTR_height:
    //         {
    //             mp_raise_msg(&mp_type_AttributeError, MP_ERROR_TEXT("Text2DNode: ERROR: 'height' is read-only, it is not allowed to be set!"));
    //         }
    //         break;
    //     }
    // }
}


/*  --- doc ---
    NAME: Line2DNode
    DESC: Simple 2D rectangle node (DO NOT USE: not fully implemented yet)
    PARAM:  [type={ref_link:Vector2}]         [name=start]                      [value={ref_link:Vector2}]
    PARAM:  [type={ref_link:Vector2}]         [name=end]                        [value={ref_link:Vector2}]
    PARAM:  [type=float]                      [name=thickness]                  [value=any]
    PARAM:  [type=int]                        [name=color]                      [value=0 ~ 65535 (16-bit RGB565 0bRRRRRGGGGGGBBBBB)]   
    PARAM:  [type=bool]                       [name=outline]                    [value=True or False]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector2}]         [name=start]                      [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:Vector2}]         [name=end]                        [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    ATTR:   [type=float]                      [name=thickness]                  [value=any]
    ATTR:   [type=int]                        [name=color]                      [value=0 ~ 65535 (16-bit RGB565 0bRRRRRGGGGGGBBBBB)]   
    ATTR:   [type=bool]                       [name=outline]                    [value=True or False]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    OVRR:   [type=function]                   [name={ref_link:draw}]            [value=function]
*/
mp_obj_t line_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Line2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_start,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_end,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_thickness,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_outline,      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, start, end, thickness, color, outline};
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

    if(parsed_args[start].u_obj == MP_OBJ_NULL) parsed_args[start].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(-5.0f)});
    if(parsed_args[end].u_obj == MP_OBJ_NULL) parsed_args[end].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(5.0f)});
    if(parsed_args[thickness].u_obj == MP_OBJ_NULL) parsed_args[thickness].u_obj = mp_obj_new_float(1.0f);
    if(parsed_args[color].u_obj == MP_OBJ_NULL) parsed_args[color].u_obj = mp_obj_new_int(0xffff);
    if(parsed_args[outline].u_obj == MP_OBJ_NULL) parsed_args[outline].u_obj = mp_obj_new_bool(false);

    engine_line_2d_node_common_data_t *common_data = malloc(sizeof(engine_line_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_line_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_LINE_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(inherited == false){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_line_2d_node_class_obj_t *line_2d_node = m_malloc(sizeof(engine_line_2d_node_class_obj_t));
        node_base->node = line_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&line_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&line_2d_node_class_draw_obj);

        line_2d_node->start = parsed_args[start].u_obj;
        line_2d_node->end = parsed_args[end].u_obj;
        line_2d_node->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        line_2d_node->thickness = parsed_args[thickness].u_obj;
        line_2d_node->color = parsed_args[color].u_obj;
        line_2d_node->outline = parsed_args[outline].u_obj;
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = parsed_args[child_class].u_obj;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&line_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&line_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_start, parsed_args[start].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_end, parsed_args[end].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_position, vector2_class_new(&vector2_class_type, 0, 0, NULL));
        mp_store_attr(node_base->node, MP_QSTR_thickness, parsed_args[thickness].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_color, parsed_args[color].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_outline, parsed_args[outline].u_obj);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run
        default_instance_attr_func = MP_OBJ_TYPE_GET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_base->node)->type, attr);
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_base->node)->type, attr, line_2d_node_class_set, 5);
    }

    return MP_OBJ_FROM_PTR(node_base);
}


STATIC void line_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Line2DNode attr");

    engine_line_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_start:
                destination[0] = self->start;
            break;
            case MP_QSTR_end:
                destination[0] = self->end;
            break;
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_thickness:
                destination[0] = self->thickness;
            break;
            case MP_QSTR_color:
                destination[0] = self->color;
            break;
            case MP_QSTR_outline:
                destination[0] = self->outline;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_start:
                self->start = destination[1];
            break;
            case MP_QSTR_end:
                self->end = destination[1];
            break;
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_thickness:
                self->thickness = destination[1];
            break;
            case MP_QSTR_color:
                self->color = destination[1];
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
STATIC const mp_rom_map_elem_t line_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(line_2d_node_class_locals_dict, line_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_line_2d_node_class_type,
    MP_QSTR_Line2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, line_2d_node_class_new,
    print, line_2d_node_class_print,
    attr, line_2d_node_class_attr,
    locals_dict, &line_2d_node_class_locals_dict
);