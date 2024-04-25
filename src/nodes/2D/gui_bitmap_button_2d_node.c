#include "gui_bitmap_button_2d_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "nodes/2D/text_2d_node.h"
#include "debug/debug_print.h"
#include "engine_object_layers.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "draw/engine_display_draw.h"
#include "draw/engine_shader.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "resources/engine_font_resource.h"
#include "engine_gui.h"
#include "input/engine_input_common.h"

#include <string.h>


void gui_bitmap_button_2d_node_class_draw(engine_node_base_t *button_node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("GUIBitmapButton2DNode: Drawing");
    
    engine_gui_bitmap_button_2d_node_class_obj_t *button = button_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float button_opacity = mp_obj_get_float(button->opacity);
    if(engine_math_compare_floats(button_opacity, 0.0f)){
        return;
    }

    if(button->bitmap_texture != mp_const_none && button->text != mp_const_none && button->font_resource != mp_const_none){
        engine_node_base_t *camera_node_base = camera_node;

        vector3_class_obj_t *camera_position = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_position);
        rectangle_class_obj_t *camera_viewport = mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_viewport);
        float camera_zoom = mp_obj_get_float(mp_load_attr(camera_node_base->attr_accessor, MP_QSTR_zoom));

        float camera_resolved_hierarchy_x = 0.0f;
        float camera_resolved_hierarchy_y = 0.0f;
        float camera_resolved_hierarchy_rotation = 0.0f;
        node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, NULL, camera_node);
        camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

        float button_resolved_hierarchy_x = 0.0f;
        float button_resolved_hierarchy_y = 0.0f;
        float button_resolved_hierarchy_rotation = 0.0f;
        bool button_is_child_of_camera = false;

        color_class_obj_t *transparent_color = button->transparent_color;

        node_base_get_child_absolute_xy(&button_resolved_hierarchy_x, &button_resolved_hierarchy_y, &button_resolved_hierarchy_rotation, &button_is_child_of_camera, button_node_base);


        // Store the non-rotated x and y for a second
        float button_rotated_x = button_resolved_hierarchy_x - camera_resolved_hierarchy_x;
        float button_rotated_y = button_resolved_hierarchy_y - camera_resolved_hierarchy_y;

        if(button_is_child_of_camera == false){
            // Scale transformation due to camera zoom
            engine_math_scale_point(&button_rotated_x, &button_rotated_y, camera_position->x.value, camera_position->y.value, camera_zoom);
        }else{
            camera_zoom = 1.0f;
        }

        // Rotate text origin about the camera
        engine_math_rotate_point(&button_rotated_x, &button_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

        button_rotated_x += camera_viewport->width/2;
        button_rotated_y += camera_viewport->height/2;

        font_resource_class_obj_t *font = button->font_resource;
        vector2_class_obj_t *button_scale = button->scale;
        vector2_class_obj_t *button_text_scale = button->text_scale;
        float button_opacity = mp_obj_get_float(button->opacity);

        float btn_x_scale = button_scale->x.value*camera_zoom;
        float btn_y_scale = button_scale->y.value*camera_zoom;

        float text_letter_spacing = mp_obj_get_float(button->letter_spacing);
        float text_line_spacing = mp_obj_get_float(button->line_spacing);

        float text_x_scale = btn_x_scale * button_text_scale->x.value;
        float text_y_scale = btn_y_scale * button_text_scale->x.value;

        // Decide which shader to use per-pixel
        engine_shader_t *shader = &empty_shader;
        if(button_opacity < 1.0f){
            shader = &opacity_shader;
        }

        texture_resource_class_obj_t *bitmap = button->bitmap_texture;
        color_class_obj_t *text_color = button->text_color;

        if(button->pressed){
            if(button->pressed_text_color != mp_const_none) text_color = button->pressed_text_color;
            if(button->pressed_bitmap_texture != mp_const_none) bitmap = button->pressed_bitmap_texture;
        }else if(button->focused){
            if(button->focused_text_color != mp_const_none) text_color = button->focused_text_color;
            if(button->focused_bitmap_texture != mp_const_none) bitmap = button->focused_bitmap_texture;
        }

        engine_draw_blit(bitmap->data,
                     floorf(button_rotated_x), floorf(button_rotated_y),
                     bitmap->width, bitmap->height,
                     bitmap->width,
                     btn_x_scale,
                     btn_y_scale,
                     -(button_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation),
                     transparent_color->value.val,
                     button_opacity,
                     shader);

        engine_shader_t *text_shader = NULL;

        if(text_color == mp_const_none){
            text_shader = &empty_shader;
        }else{
            text_shader = &blend_opacity_shader;

            float t = 1.0f;

            blend_opacity_shader.program[1] = (text_color->value.val >> 8) & 0b11111111;
            blend_opacity_shader.program[2] = (text_color->value.val >> 0) & 0b11111111;

            memcpy(blend_opacity_shader.program+3, &t, sizeof(float));
        }

        engine_draw_text(font, button->text,
                         floorf(button_rotated_x), floorf(button_rotated_y),
                         button->text_width, button->text_height,
                         text_letter_spacing,
                         text_line_spacing,
                         text_x_scale, text_y_scale,
                         button_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation,
                         button_opacity,
                         text_shader);
    }
}


void gui_bitmap_button_2d_node_calculate_dimensions(engine_gui_bitmap_button_2d_node_class_obj_t *button){
    if(button->text != mp_const_none && button->font_resource != mp_const_none){
        font_resource_get_box_dimensions(button->font_resource, button->text, &button->text_width, &button->text_height, mp_obj_get_float(button->letter_spacing), mp_obj_get_float(button->line_spacing));
    }
}


mp_obj_t gui_bitmap_button_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("GUIBitmapButton2DNode: Deleted (garbage collected, removing self from active gui objects)");

    engine_node_base_t *node_base = self_in;
    engine_gui_bitmap_button_2d_node_class_obj_t *gui_button = node_base->node;
    engine_gui_untrack(gui_button->gui_list_node);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gui_bitmap_button_2d_node_class_del_obj, gui_bitmap_button_2d_node_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool bitmap_button_2d_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_gui_bitmap_button_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&gui_bitmap_button_2d_node_class_del_obj);
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
        case MP_QSTR_node_base:
            destination[0] = self_node_base;
            return true;
        break;
        case MP_QSTR_position:
            destination[0] = self->position;
            return true;
        break;
        case MP_QSTR_font:
            destination[0] = self->font_resource;
            return true;
        break;
        case MP_QSTR_text:
            destination[0] = self->text;
            return true;
        break;

        case MP_QSTR_text_color:
            destination[0] = self->text_color;
            return true;
        break;
        case MP_QSTR_focused_text_color:
            destination[0] = self->focused_text_color;
            return true;
        break;
        case MP_QSTR_pressed_text_color:
            destination[0] = self->pressed_text_color;
            return true;
        break;

        case MP_QSTR_bitmap:
            destination[0] = self->bitmap_texture;
            return true;
        break;
        case MP_QSTR_focused_bitmap:
            destination[0] = self->focused_bitmap_texture;
            return true;
        break;
        case MP_QSTR_pressed_bitmap:
            destination[0] = self->pressed_bitmap_texture;
            return true;
        break;

        case MP_QSTR_transparent_color:
            destination[0] = self->transparent_color;
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
        case MP_QSTR_text_scale:
            destination[0] = self->text_scale;
            return true;
        break;
        case MP_QSTR_opacity:
            destination[0] = self->opacity;
            return true;
        break;

        case MP_QSTR_letter_spacing:
            destination[0] = self->letter_spacing;
            return true;
        break;
        case MP_QSTR_line_spacing:
            destination[0] = self->line_spacing;
            return true;
        break;

        case MP_QSTR_tick:
            destination[0] = self->tick_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;

        case MP_QSTR_on_focused:
            destination[0] = self->on_focused_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_on_just_focused:
            destination[0] = self->on_just_focused_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_on_just_unfocused:
            destination[0] = self->on_just_unfocused_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;

        case MP_QSTR_on_pressed:
            destination[0] = self->on_pressed_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_on_just_pressed:
            destination[0] = self->on_just_pressed_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;
        case MP_QSTR_on_just_released:
            destination[0] = self->on_just_released_cb;
            destination[1] = self_node_base->attr_accessor;
            return true;
        break;

        case MP_QSTR_focused:
            destination[0] = mp_obj_new_bool(self->focused);
            return true;
        break;
        case MP_QSTR_pressed:
            destination[0] = mp_obj_new_bool(self->pressed);
            return true;
        break;
        case MP_QSTR_button:
            destination[0] = mp_obj_new_int(self->button);
            return true;
        break;
        case MP_QSTR_width:
            // destination[0] = mp_obj_new_float(self->text_width);
            return true;
        break;
        case MP_QSTR_height:
            // destination[0] = mp_obj_new_float(self->text_height);
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool bitmap_button_2d_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_gui_bitmap_button_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_font:
            self->font_resource = destination[1];
            gui_bitmap_button_2d_node_calculate_dimensions(self);
            return true;
        break;
        case MP_QSTR_text:
            self->text = destination[1];
            gui_bitmap_button_2d_node_calculate_dimensions(self);
            return true;
        break;

        case MP_QSTR_text_color:
            self->text_color = destination[1];
            return true;
        break;
        case MP_QSTR_focused_text_color:
            self->focused_text_color = destination[1];
            return true;
        break;
        case MP_QSTR_pressed_text_color:
            self->pressed_text_color = destination[1];
            return true;
        break;

        case MP_QSTR_bitmap:
            self->bitmap_texture = destination[1];
            return true;
        break;
        case MP_QSTR_focused_bitmap:
            self->focused_bitmap_texture = destination[1];
            return true;
        break;
        case MP_QSTR_pressed_bitmap:
            self->pressed_bitmap_texture = destination[1];
            return true;
        break;

        case MP_QSTR_transparent_color:
            self->transparent_color = destination[1];
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
        case MP_QSTR_text_scale:
            self->text_scale = destination[1];
            return true;
        break;
        case MP_QSTR_opacity:
            self->opacity = destination[1];
            return true;
        break;

        case MP_QSTR_letter_spacing:
            self->letter_spacing = destination[1];
            return true;
        break;
        case MP_QSTR_line_spacing:
            self->line_spacing = destination[1];
            return true;
        break;

        case MP_QSTR_tick:
            self->tick_cb = destination[1];
            return true;
        break;

        case MP_QSTR_on_focused:
            self->on_focused_cb = destination[1];
            return true;
        break;
        case MP_QSTR_on_just_focused:
            self->on_just_focused_cb = destination[1];
            return true;
        break;
        case MP_QSTR_on_just_unfocused:
            self->on_just_unfocused_cb = destination[1];
            return true;
        break;
        
        case MP_QSTR_on_pressed:
            self->on_pressed_cb = destination[1];
            return true;
        break;
        case MP_QSTR_on_just_pressed:
            self->on_just_pressed_cb = destination[1];
            return true;
        break;
        case MP_QSTR_on_just_released:
            self->on_just_released_cb = destination[1];
            return true;
        break;

        case MP_QSTR_focused:
            self->focused = mp_obj_get_int(destination[1]);
            if(self->focused == true) engine_gui_focus_node(self_node_base);
            return true;
        break;
        case MP_QSTR_pressed:
            self->pressed = mp_obj_get_int(destination[1]);
            return true;
        break;
        case MP_QSTR_button:
            self->button = mp_obj_get_int(destination[1]);
            return true;
        break;
        case MP_QSTR_width:
            mp_raise_msg(&mp_type_AttributeError, MP_ERROR_TEXT("GUIBitmapButton2DNode: ERROR: 'width' is read-only, it is not allowed to be set!"));
            return true;
        break;
        case MP_QSTR_height:
            mp_raise_msg(&mp_type_AttributeError, MP_ERROR_TEXT("GUIBitmapButton2DNode: ERROR: 'height' is read-only, it is not allowed to be set!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t gui_bitmap_button_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing GUIBitmapButton2DNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = bitmap_button_2d_node_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = bitmap_button_2d_node_store_attr(node_base, attribute, destination);

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
    NAME: GUIBitmapButton2DNode
    DESC: 
*/
mp_obj_t gui_bitmap_button_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New GUIBitmapButton2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,                  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,                     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_font,                         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_text,                         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_text_color,                   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_focused_text_color,           MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pressed_text_color,           MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_bitmap,                       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_focused_bitmap,               MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pressed_bitmap,               MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_transparent_color,    MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_rotation,                     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,                        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_text_scale,                   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_opacity,                      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_letter_spacing,       MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_line_spacing,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} }
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, font, text,

                  text_color,
                  focused_text_color,
                  pressed_text_color,
                  
                  bitmap,
                  focused_bitmap,
                  pressed_bitmap,

                  transparent_color,

                  rotation, scale, text_scale, opacity,
                  
                  letter_spacing,
                  line_spacing};

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

    if(parsed_args[position].u_obj == MP_OBJ_NULL) parsed_args[position].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)});
    if(parsed_args[font].u_obj == MP_OBJ_NULL) parsed_args[font].u_obj = mp_const_none;
    if(parsed_args[text].u_obj == MP_OBJ_NULL) parsed_args[text].u_obj = mp_const_none;

    if(parsed_args[text_color].u_obj == MP_OBJ_NULL) parsed_args[text_color].u_obj = mp_const_none;
    if(parsed_args[focused_text_color].u_obj == MP_OBJ_NULL) parsed_args[focused_text_color].u_obj = mp_const_none;
    if(parsed_args[pressed_text_color].u_obj == MP_OBJ_NULL) parsed_args[pressed_text_color].u_obj = mp_const_none;

    if(parsed_args[bitmap].u_obj == MP_OBJ_NULL) parsed_args[bitmap].u_obj = mp_const_none;
    if(parsed_args[focused_bitmap].u_obj == MP_OBJ_NULL) parsed_args[focused_bitmap].u_obj = mp_const_none;
    if(parsed_args[pressed_bitmap].u_obj == MP_OBJ_NULL) parsed_args[pressed_bitmap].u_obj = mp_const_none;

    if(parsed_args[transparent_color].u_obj == MP_OBJ_NULL) parsed_args[transparent_color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(ENGINE_NO_TRANSPARENCY_COLOR)});

    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});
    if(parsed_args[text_scale].u_obj == MP_OBJ_NULL) parsed_args[text_scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});
    if(parsed_args[opacity].u_obj == MP_OBJ_NULL) parsed_args[opacity].u_obj = mp_obj_new_float(1.0f);

    if(parsed_args[letter_spacing].u_obj == MP_OBJ_NULL) parsed_args[letter_spacing].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[line_spacing].u_obj == MP_OBJ_NULL) parsed_args[line_spacing].u_obj = mp_obj_new_float(0.0f);

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, &engine_gui_bitmap_button_2d_node_class_type, NODE_TYPE_GUI_BITMAP_BUTTON_2D);
    engine_gui_bitmap_button_2d_node_class_obj_t *gui_bitmap_button_2d_node = m_malloc(sizeof(engine_gui_bitmap_button_2d_node_class_obj_t));
    node_base->node = gui_bitmap_button_2d_node;
    node_base->attr_accessor = node_base;

    gui_bitmap_button_2d_node->gui_list_node = engine_gui_track(node_base);

    gui_bitmap_button_2d_node->tick_cb = mp_const_none;
    gui_bitmap_button_2d_node->on_focused_cb = mp_const_none;
    gui_bitmap_button_2d_node->on_just_focused_cb = mp_const_none;
    gui_bitmap_button_2d_node->on_just_unfocused_cb = mp_const_none;
    gui_bitmap_button_2d_node->on_pressed_cb = mp_const_none;
    gui_bitmap_button_2d_node->on_just_pressed_cb = mp_const_none;
    gui_bitmap_button_2d_node->on_just_released_cb = mp_const_none;

    gui_bitmap_button_2d_node->position = parsed_args[position].u_obj;
    gui_bitmap_button_2d_node->font_resource = parsed_args[font].u_obj;
    gui_bitmap_button_2d_node->text = parsed_args[text].u_obj;

    gui_bitmap_button_2d_node->text_color = parsed_args[text_color].u_obj;
    gui_bitmap_button_2d_node->focused_text_color = parsed_args[focused_text_color].u_obj;
    gui_bitmap_button_2d_node->pressed_text_color = parsed_args[pressed_text_color].u_obj;

    gui_bitmap_button_2d_node->bitmap_texture = parsed_args[bitmap].u_obj;
    gui_bitmap_button_2d_node->focused_bitmap_texture = parsed_args[focused_bitmap].u_obj;
    gui_bitmap_button_2d_node->pressed_bitmap_texture = parsed_args[pressed_bitmap].u_obj;

    gui_bitmap_button_2d_node->transparent_color = parsed_args[transparent_color].u_obj;

    gui_bitmap_button_2d_node->rotation = parsed_args[rotation].u_obj;
    gui_bitmap_button_2d_node->scale = parsed_args[scale].u_obj;
    gui_bitmap_button_2d_node->text_scale = parsed_args[text_scale].u_obj;
    gui_bitmap_button_2d_node->opacity = parsed_args[opacity].u_obj;

    gui_bitmap_button_2d_node->letter_spacing = parsed_args[letter_spacing].u_obj;
    gui_bitmap_button_2d_node->line_spacing = parsed_args[line_spacing].u_obj;

    gui_bitmap_button_2d_node->focused = false;
    gui_bitmap_button_2d_node->pressed = false;
    gui_bitmap_button_2d_node->last_pressed = false;
    gui_bitmap_button_2d_node->button = BUTTON_A;

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Because the instance doesn't have a `node_base` yet, restore the
        // instance type original attr function for now (otherwise get core abort)
        if(default_instance_attr_func != NULL) MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, default_instance_attr_func, 5);

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];

        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_focused, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->on_focused_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->on_focused_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_focused, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->on_just_focused_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->on_just_focused_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_unfocused, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->on_just_unfocused_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->on_just_unfocused_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_pressed, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->on_pressed_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->on_pressed_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_pressed, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->on_just_pressed_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->on_just_pressed_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_released, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_bitmap_button_2d_node->on_just_released_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_bitmap_button_2d_node->on_just_released_cb = dest[0];
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
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, gui_bitmap_button_2d_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    gui_bitmap_button_2d_node_calculate_dimensions(gui_bitmap_button_2d_node);

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
STATIC const mp_rom_map_elem_t gui_bitmap_button_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(gui_bitmap_button_2d_node_class_locals_dict, gui_bitmap_button_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_gui_bitmap_button_2d_node_class_type,
    MP_QSTR_GUIBitmapButton2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, gui_bitmap_button_2d_node_class_new,
    attr, gui_bitmap_button_2d_node_class_attr,
    locals_dict, &gui_bitmap_button_2d_node_class_locals_dict
);