#include "text_2d_node.h"

#include "py/objstr.h"
#include "nodes/node_types.h"
#include "nodes/node_base.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "resources/engine_texture_resource.h"
#include "resources/engine_font_resource.h"
#include "utility/engine_file.h"
#include "math/engine_math.h"
#include "utility/engine_time.h"


// Class required functions
STATIC void text_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Text2DNode");
}


STATIC mp_obj_t text_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("Text2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(text_2d_node_class_tick_obj, text_2d_node_class_tick);


STATIC mp_obj_t text_2d_node_class_draw(mp_obj_t self_in, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("Text2DNode: Drawing");

    // Very first thing is to early out if the text is not set
    engine_node_base_t *text_node_base = self_in;
    mp_obj_t text_obj = mp_load_attr(text_node_base->attr_accessor, MP_QSTR_text);
    
    if(text_obj == mp_const_none){
        return mp_const_none;
    }

    engine_node_base_t *camera_node_base = camera_node;
    engine_text_2d_node_common_data_t *text_common_data = text_node_base->node_common_data;

    vector2_class_obj_t *text_scale =  mp_load_attr(text_node_base->attr_accessor, MP_QSTR_scale);
    font_resource_class_obj_t *text_font = mp_load_attr(text_node_base->attr_accessor, MP_QSTR_font);
    uint16_t text_font_bitmap_width = text_font->texture_resource->width;

    uint16_t *text_pixel_data = (uint16_t*)text_font->texture_resource->data;
    uint8_t char_height = text_font->glyph_height;

    vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_rotation);
    vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
    rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
    float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

    float camera_resolved_hierarchy_x = 0.0f;
    float camera_resolved_hierarchy_y = 0.0f;
    float camera_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, camera_node);
    camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

    float text_resolved_hierarchy_x = 0.0f;
    float text_resolved_hierarchy_y = 0.0f;
    float text_resolved_hierarchy_rotation = 0.0f;
    node_base_get_child_absolute_xy(&text_resolved_hierarchy_x, &text_resolved_hierarchy_y, &text_resolved_hierarchy_rotation, self_in);

    // Store the non-rotated x and y for a second
    float text_rotated_x = text_resolved_hierarchy_x-camera_resolved_hierarchy_x;
    float text_rotated_y = text_resolved_hierarchy_y-camera_resolved_hierarchy_y;

    // Scale transformation due to camera zoom
    engine_math_scale_point(&text_rotated_x, &text_rotated_y, camera_position->x, camera_position->y, camera_zoom);

    // Rotate text origin about the camera
    engine_math_rotate_point(&text_rotated_x, &text_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

    text_rotated_x += camera_viewport->width/2;
    text_rotated_y += camera_viewport->height/2;

    // Get length of string: https://github.com/v923z/micropython-usermod/blob/master/snippets/stringarg/stringarg.c
    GET_STR_DATA_LEN(text_obj, str, str_len);

    float char_center_x = text_rotated_x;
    float char_center_y = text_rotated_y;

    float x_scale = text_scale->x*camera_zoom;
    float y_scale = text_scale->y*camera_zoom;

    for(uint16_t icx=0; icx<str_len; icx++){
        char current_char = ((char *)str)[icx];

        uint8_t char_width = font_resource_get_glyph_width(text_font, current_char);
        uint8_t char_x_offset = font_resource_get_glyph_x_offset(text_font, current_char);

        engine_draw_blit(text_pixel_data+engine_math_2d_to_1d_index(char_x_offset, 0, text_font_bitmap_width),
                        char_center_x, char_center_y,
                        char_width, char_height-1,
                        text_font_bitmap_width,
                        x_scale,
                        y_scale,
                        0.0f);

        // engine_draw_blit_scale_rotate(  text_pixel_data+engine_math_2d_to_1d_index(char_x_offset, 0, text_font_bitmap_width),
        //                                 (int32_t)char_center_x,
        //                                 (int32_t)char_center_y,
        //                                 text_font_bitmap_width,
        //                                 char_width,
        //                                 char_height-1,
        //                                 (int32_t)(x_scale*65536 + 0.5),
        //                                 (int32_t)(y_scale*65536 + 0.5),
        //                                 (int16_t)(((text_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation))*1024 / (float)(2*PI)),
        //                                 0x0000);
        
        char_center_x += char_width * x_scale;
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(text_2d_node_class_draw_obj, text_2d_node_class_draw);


/*  --- doc ---
    NAME: Text2DNode
    DESC: Simple 2D sprite node that displays text
    PARAM:  [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    PARAM:  [type={ref_link:FontResource}]    [name=font]                       [value={ref_link:FontResource}]
    PARAM:  [type=string]                     [name=text]                       [value=any]
    PARAM:  [type=float]                      [name=rotation]                   [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    ATTR:   [type=function]                   [name={ref_link:add_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:get_child}]       [value=function] 
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type=function]                   [name={ref_link:set_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:get_layer}]       [value=function]
    ATTR:   [type=function]                   [name={ref_link:remove_child}]    [value=function]
    ATTR:   [type={ref_link:Vector2}]         [name=position]                   [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:FontResource}]    [name=font]                       [value={ref_link:FontResource}]
    ATTR:   [type=string]                     [name=text]                       [value=any]
    ATTR:   [type=float]                      [name=rotation]                   [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]         [name=scale]                      [value={ref_link:Vector2}]
    OVRR:   [type=function]                   [name={ref_link:tick}]            [value=function]
    OVRR:   [type=function]                   [name={ref_link:draw}]            [value=function]
*/
mp_obj_t text_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Text2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,          MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_font,                 MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_text,                 MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_rotation,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,                MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, font, text, rotation, scale};
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
    if(parsed_args[font].u_obj == MP_OBJ_NULL) parsed_args[font].u_obj = mp_const_none;
    if(parsed_args[text].u_obj == MP_OBJ_NULL) parsed_args[text].u_obj = mp_const_none;
    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});

    engine_text_2d_node_common_data_t *common_data = malloc(sizeof(engine_text_2d_node_common_data_t));

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base->node_common_data = common_data;
    node_base->base.type = &engine_text_2d_node_class_type;
    node_base->layer = 0;
    node_base->type = NODE_TYPE_TEXT_2D;
    node_base->object_list_node = engine_add_object_to_layer(node_base, node_base->layer);
    node_base_set_if_visible(node_base, true);
    node_base_set_if_disabled(node_base, false);
    node_base_set_if_just_added(node_base, true);

    if(inherited == false){        // Non-inherited (create a new object)
        node_base->inherited = false;

        engine_text_2d_node_class_obj_t *text_2d_node = m_malloc(sizeof(engine_text_2d_node_class_obj_t));
        node_base->node = text_2d_node;
        node_base->attr_accessor = node_base;

        common_data->tick_cb = MP_OBJ_FROM_PTR(&text_2d_node_class_tick_obj);
        common_data->draw_cb = MP_OBJ_FROM_PTR(&text_2d_node_class_draw_obj);

        text_2d_node->position = parsed_args[position].u_obj;
        text_2d_node->font_resource = parsed_args[font].u_obj;
        text_2d_node->text = parsed_args[text].u_obj;
        text_2d_node->rotation = parsed_args[rotation].u_obj;
        text_2d_node->scale = parsed_args[scale].u_obj;
    }else if(inherited == true){  // Inherited (use existing object)
        node_base->inherited = true;
        node_base->node = parsed_args[child_class].u_obj;;
        node_base->attr_accessor = node_base->node;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_base->node, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->tick_cb = MP_OBJ_FROM_PTR(&text_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_base->node, MP_QSTR_draw, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            common_data->draw_cb = MP_OBJ_FROM_PTR(&text_2d_node_class_draw_obj);
        }else{                                                  // Likely found method (could be attribute)
            common_data->draw_cb = dest[0];
        }

        mp_store_attr(node_base->node, MP_QSTR_position, parsed_args[position].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_font, parsed_args[font].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_text, parsed_args[text].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_rotation, parsed_args[rotation].u_obj);
        mp_store_attr(node_base->node, MP_QSTR_scale, parsed_args[scale].u_obj);
    }

    return MP_OBJ_FROM_PTR(node_base);
}


STATIC void text_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Text2DNode attr");

    engine_text_2d_node_class_obj_t *self = ((engine_node_base_t*)(self_in))->node;

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
            case MP_QSTR_font:
                destination[0] = self->font_resource;
            break;
            case MP_QSTR_text:
                destination[0] = self->text;
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
            case MP_QSTR_font:
                self->font_resource = destination[1];
            break;
            case MP_QSTR_text:
                self->text = destination[1];
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
STATIC const mp_rom_map_elem_t text_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(text_2d_node_class_locals_dict, text_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_text_2d_node_class_type,
    MP_QSTR_Text2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, text_2d_node_class_new,
    print, text_2d_node_class_print,
    attr, text_2d_node_class_attr,
    locals_dict, &text_2d_node_class_locals_dict
);