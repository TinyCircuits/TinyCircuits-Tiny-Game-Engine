#include "gui_button_2d_node.h"

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

#include <string.h>


// Class required functions
STATIC void gui_button_2d_node_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_PRINTF("GUIButton2DNode");
}


STATIC mp_obj_t gui_button_2d_node_class_tick(mp_obj_t self_in){
    ENGINE_WARNING_PRINTF("GUIButton2DNode: Tick function not overridden");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(gui_button_2d_node_class_tick_obj, gui_button_2d_node_class_tick);


void gui_button_2d_node_class_draw(engine_node_base_t *button_node_base, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("GUIButton2DNode: Drawing");
    
    engine_gui_button_2d_node_class_obj_t *button = button_node_base->node;

    if(button->text != mp_const_none && button->font_resource != mp_const_none){
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
        float button_opacity = mp_obj_get_float(button->opacity);

        float x_scale = button_scale->x.value*camera_zoom;
        float y_scale = button_scale->y.value*camera_zoom;

        // Decide which shader to use per-pixel
        engine_shader_t *shader = &empty_shader;
        if(button_opacity < 1.0f){
            shader = &opacity_shader;
        }

        color_class_obj_t *outline_color = button->outline_color;
        color_class_obj_t *background_color = button->background_color;

        if(button->focused){
            if(button->focused_outline_color != mp_const_none) outline_color = button->focused_outline_color;
            if(button->focused_background_color != mp_const_none) background_color = button->focused_background_color;
        }

        engine_draw_rect(outline_color->value.val,
                         button_rotated_x, button_rotated_y,
                         button->width_outline, button->height_outline,
                         x_scale, y_scale,
                       -(button_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation),
                         button_opacity,
                         shader);

        engine_draw_rect(background_color->value.val,
                         button_rotated_x, button_rotated_y,
                         button->width_padded, button->height_padded,
                         x_scale, y_scale,
                       -(button_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation),
                         button_opacity,
                         shader);


        engine_shader_t *text_shader = NULL;

        if(button->text_color == mp_const_none){
            text_shader = &empty_shader;
        }else{
            color_class_obj_t *text_color = button->text_color;
            text_shader = &blend_opacity_shader;

            float t = 1.0f;

            blend_opacity_shader.program[1] = (text_color->value.val >> 8) & 0b11111111;
            blend_opacity_shader.program[2] = (text_color->value.val >> 0) & 0b11111111;

            memcpy(blend_opacity_shader.program+3, &t, sizeof(float));
        }

        engine_draw_text(font, button->text,
                         button_rotated_x, button_rotated_y,
                         button->width, button->height,
                         x_scale, y_scale,
                         button_resolved_hierarchy_rotation+camera_resolved_hierarchy_rotation,
                         button_opacity,
                         text_shader);
    }
}


void gui_button_2d_node_calculate_dimensions(engine_gui_button_2d_node_class_obj_t *button){
    if(button->text != mp_const_none && button->font_resource != mp_const_none){
        font_resource_get_box_dimensions(button->font_resource, button->text, &button->width, &button->height);
        float padding = mp_obj_get_float(button->padding) * 2.0f;
        float outline = mp_obj_get_float(button->outline) * 2.0f;
        button->width_padded = button->width + padding;
        button->height_padded = button->height + padding;
        button->width_outline = button->width_padded + outline;
        button->height_outline = button->height_padded + outline;
    }
}


mp_obj_t gui_button_2d_node_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("GUIButton2DNode: Deleted (garbage collected, removing self from active gui objects)");

    engine_node_base_t *node_base = self_in;
    engine_gui_button_2d_node_class_obj_t *gui_button = node_base->node;
    engine_gui_untrack(gui_button->gui_list_node);

    node_base_del(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(gui_button_2d_node_class_del_obj, gui_button_2d_node_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool button_2d_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_gui_button_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&gui_button_2d_node_class_del_obj);
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
        case MP_QSTR_outline:
            destination[0] = self->outline;
            return true;
        break;
        case MP_QSTR_padding:
            destination[0] = self->padding;
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

        case MP_QSTR_background_color:
            destination[0] = self->background_color;
            return true;
        break;
        case MP_QSTR_focused_background_color:
            destination[0] = self->focused_background_color;
            return true;
        break;
        case MP_QSTR_pressed_background_color:
            destination[0] = self->pressed_background_color;
            return true;
        break;

        case MP_QSTR_outline_color:
            destination[0] = self->outline_color;
            return true;
        break;
        case MP_QSTR_focused_outline_color:
            destination[0] = self->focused_outline_color;
            return true;
        break;
        case MP_QSTR_pressed_outline_color:
            destination[0] = self->pressed_outline_color;
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
        case MP_QSTR_opacity:
            destination[0] = self->opacity;
            return true;
        break;
        case MP_QSTR_focused:
            destination[0] = mp_obj_new_bool(self->focused);
            return true;
        break;
        case MP_QSTR_width:
            destination[0] = mp_obj_new_float(self->width_outline);
            return true;
        break;
        case MP_QSTR_height:
            destination[0] = mp_obj_new_float(self->height_outline);
            return true;
        break;
        default:
            return false; // Fail
    }
}


// Return `true` if handled storing the attr from internal structure, `false` otherwise
bool button_2d_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_gui_button_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR_position:
            self->position = destination[1];
            return true;
        break;
        case MP_QSTR_font:
            self->font_resource = destination[1];
            gui_button_2d_node_calculate_dimensions(self);
            return true;
        break;
        case MP_QSTR_text:
            self->text = destination[1];
            gui_button_2d_node_calculate_dimensions(self);
            return true;
        break;
        case MP_QSTR_outline:
            self->outline = destination[1];
            gui_button_2d_node_calculate_dimensions(self);
            return true;
        break;
        case MP_QSTR_padding:
            self->padding = destination[1];
            gui_button_2d_node_calculate_dimensions(self);
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

        case MP_QSTR_background_color:
            self->background_color = destination[1];
            return true;
        break;
        case MP_QSTR_focused_background_color:
            self->focused_background_color = destination[1];
            return true;
        break;
        case MP_QSTR_pressed_background_color:
            self->pressed_background_color = destination[1];
            return true;
        break;

        case MP_QSTR_outline_color:
            self->outline_color = destination[1];
            return true;
        break;
        case MP_QSTR_focused_outline_color:
            self->focused_outline_color = destination[1];
            return true;
        break;
        case MP_QSTR_pressed_outline_color:
            self->pressed_outline_color = destination[1];
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
        case MP_QSTR_opacity:
            self->opacity = destination[1];
            return true;
        break;
        case MP_QSTR_focused:
            self->focused = mp_obj_get_int(destination[1]);
            return true;
        break;
        case MP_QSTR_width:
            mp_raise_msg(&mp_type_AttributeError, MP_ERROR_TEXT("GUIButton2DNode: ERROR: 'width' is read-only, it is not allowed to be set!"));
            return true;
        break;
        case MP_QSTR_height:
            mp_raise_msg(&mp_type_AttributeError, MP_ERROR_TEXT("GUIButton2DNode: ERROR: 'height' is read-only, it is not allowed to be set!"));
            return true;
        break;
        default:
            return false; // Fail
    }
}


STATIC mp_attr_fun_t gui_button_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing GUIButton2DNode attr");

    // Get the node base from either class
    // instance or native instance object
    bool is_obj_instance = false;
    engine_node_base_t *node_base = node_base_get(self_in, &is_obj_instance);

    // Used for telling if custom load/store functions handled the attr
    bool attr_handled = false;

    if(destination[0] == MP_OBJ_NULL){          // Load
        attr_handled = button_2d_load_attr(node_base, attribute, destination);
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        attr_handled = button_2d_store_attr(node_base, attribute, destination);

        // If handled, mark as successful store
        if(attr_handled) destination[0] = MP_OBJ_NULL;
    }

    // If this is a Python class instance and the attr was NOT
    // handled by the above, defer the attr to the instance attr
    // handler
    if(is_obj_instance && attr_handled == false){
        default_instance_attr_func(self_in, attribute, destination);
    }
}


/*  --- doc ---
    NAME: GUIButton2DNode
    DESC: 
*/
mp_obj_t gui_button_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New GUIButton2DNode");

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,                  MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,                     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_font,                         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_text,                         MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_outline,                      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_padding,                      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_text_color,                   MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_focused_text_color,           MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pressed_text_color,           MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_background_color,             MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_focused_background_color,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pressed_background_color,     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_outline_color,                MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_focused_outline_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_pressed_outline_color,        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },

        { MP_QSTR_rotation,                     MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scale,                        MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_opacity,                      MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    enum arg_ids {child_class, position, font, text, outline, padding,

                  text_color,
                  focused_text_color,
                  pressed_text_color,

                  background_color,
                  focused_background_color,
                  pressed_background_color,

                  outline_color,
                  focused_outline_color,
                  pressed_outline_color,

                  rotation, scale, opacity};
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
    if(parsed_args[outline].u_obj == MP_OBJ_NULL) parsed_args[outline].u_obj = mp_obj_new_float(2.0f);
    if(parsed_args[padding].u_obj == MP_OBJ_NULL) parsed_args[padding].u_obj = mp_obj_new_float(2.0f);

    if(parsed_args[text_color].u_obj == MP_OBJ_NULL) parsed_args[text_color].u_obj = mp_const_none;
    if(parsed_args[focused_text_color].u_obj == MP_OBJ_NULL) parsed_args[focused_text_color].u_obj = mp_const_none;
    if(parsed_args[pressed_text_color].u_obj == MP_OBJ_NULL) parsed_args[pressed_text_color].u_obj = mp_const_none;

    if(parsed_args[background_color].u_obj == MP_OBJ_NULL) parsed_args[background_color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x7bef)});
    if(parsed_args[focused_background_color].u_obj == MP_OBJ_NULL) parsed_args[focused_background_color].u_obj = mp_const_none;
    if(parsed_args[pressed_background_color].u_obj == MP_OBJ_NULL) parsed_args[pressed_background_color].u_obj = mp_const_none;

    if(parsed_args[outline_color].u_obj == MP_OBJ_NULL) parsed_args[outline_color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xa554)});
    if(parsed_args[focused_outline_color].u_obj == MP_OBJ_NULL) parsed_args[focused_outline_color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xff40)});
    if(parsed_args[pressed_outline_color].u_obj == MP_OBJ_NULL) parsed_args[pressed_outline_color].u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xde60)});

    if(parsed_args[rotation].u_obj == MP_OBJ_NULL) parsed_args[rotation].u_obj = mp_obj_new_float(0.0f);
    if(parsed_args[scale].u_obj == MP_OBJ_NULL) parsed_args[scale].u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)});
    if(parsed_args[opacity].u_obj == MP_OBJ_NULL) parsed_args[opacity].u_obj = mp_obj_new_float(1.0f);

    // All nodes are a engine_node_base_t node. Specific node data is stored in engine_node_base_t->node
    engine_node_base_t *node_base = m_new_obj_with_finaliser(engine_node_base_t);
    node_base_init(node_base, NULL, &engine_gui_button_2d_node_class_type, NODE_TYPE_GUI_BUTTON_2D);
    engine_gui_button_2d_node_class_obj_t *gui_button_2d_node = m_malloc(sizeof(engine_gui_button_2d_node_class_obj_t));
    node_base->node = gui_button_2d_node;
    node_base->attr_accessor = node_base;

    gui_button_2d_node->gui_list_node = engine_gui_track(node_base);

    gui_button_2d_node->tick_cb = MP_OBJ_FROM_PTR(&gui_button_2d_node_class_tick_obj);
    gui_button_2d_node->position = parsed_args[position].u_obj;
    gui_button_2d_node->font_resource = parsed_args[font].u_obj;
    gui_button_2d_node->text = parsed_args[text].u_obj;
    gui_button_2d_node->outline = parsed_args[outline].u_obj;
    gui_button_2d_node->padding = parsed_args[padding].u_obj;

    gui_button_2d_node->text_color = parsed_args[text_color].u_obj;
    gui_button_2d_node->focused_text_color = parsed_args[focused_text_color].u_obj;
    gui_button_2d_node->pressed_text_color = parsed_args[pressed_text_color].u_obj;

    gui_button_2d_node->background_color = parsed_args[background_color].u_obj;
    gui_button_2d_node->focused_background_color = parsed_args[focused_background_color].u_obj;
    gui_button_2d_node->pressed_background_color = parsed_args[pressed_background_color].u_obj;

    gui_button_2d_node->outline_color = parsed_args[outline_color].u_obj;
    gui_button_2d_node->focused_outline_color = parsed_args[focused_outline_color].u_obj;
    gui_button_2d_node->pressed_outline_color = parsed_args[pressed_outline_color].u_obj;

    gui_button_2d_node->rotation = parsed_args[rotation].u_obj;
    gui_button_2d_node->scale = parsed_args[scale].u_obj;
    gui_button_2d_node->opacity = parsed_args[opacity].u_obj;
    gui_button_2d_node->focused = false;

    if(inherited == true){
        // Get the Python class instance
        mp_obj_t node_instance = parsed_args[child_class].u_obj;

        // Look for function overrides otherwise use the defaults
        mp_obj_t dest[2];
        mp_load_method_maybe(node_instance, MP_QSTR_tick, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->tick_cb = MP_OBJ_FROM_PTR(&gui_button_2d_node_class_tick_obj);
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->tick_cb = dest[0];
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
        MP_OBJ_TYPE_SET_SLOT((mp_obj_type_t*)((mp_obj_base_t*)node_instance)->type, attr, gui_button_2d_node_class_attr, 5);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    gui_button_2d_node_calculate_dimensions(gui_button_2d_node);

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
STATIC const mp_rom_map_elem_t gui_button_2d_node_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(gui_button_2d_node_class_locals_dict, gui_button_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_gui_button_2d_node_class_type,
    MP_QSTR_GUIButton2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, gui_button_2d_node_class_new,
    print, gui_button_2d_node_class_print,
    attr, gui_button_2d_node_class_attr,
    locals_dict, &gui_button_2d_node_class_locals_dict
);