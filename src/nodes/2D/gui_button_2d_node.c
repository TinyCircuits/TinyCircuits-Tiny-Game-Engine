#include "gui_button_2d_node.h"

#include "py/objstr.h"
#include "py/objtype.h"
#include "nodes/node_types.h"
#include "nodes/2D/text_2d_node.h"
#include "nodes/3D/camera_node.h"
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
#include "io/engine_io_module.h"
#include "engine_collections.h"

#include <string.h>


void gui_button_2d_node_class_draw(mp_obj_t button_node_base_obj, mp_obj_t camera_node){
    ENGINE_INFO_PRINTF("GUIButton2DNode: Drawing");

    engine_node_base_t *button_node_base = button_node_base_obj;
    engine_gui_button_2d_node_class_obj_t *button = button_node_base->node;

    // Avoid drawing or doing anything if opacity is zero
    float button_opacity = mp_obj_get_float(button->opacity);
    if(engine_math_compare_floats(button_opacity, 0.0f)){
        return;
    }

    if(button->text != mp_const_none && button->font_resource != mp_const_none){
        engine_node_base_t *camera_node_base = camera_node;
        engine_camera_node_class_obj_t *camera = camera_node_base->node;


        rectangle_class_obj_t *camera_viewport = camera->viewport;
        float camera_zoom = mp_obj_get_float(camera->zoom);

        float button_resolved_hierarchy_x = 0.0f;
        float button_resolved_hierarchy_y = 0.0f;
        float button_resolved_hierarchy_rotation = 0.0f;
        bool button_is_child_of_camera = false;

        node_base_get_child_absolute_xy(&button_resolved_hierarchy_x, &button_resolved_hierarchy_y, &button_resolved_hierarchy_rotation, &button_is_child_of_camera, button_node_base);

        // Store the non-rotated x and y for a second
        float button_rotated_x = button_resolved_hierarchy_x;
        float button_rotated_y = button_resolved_hierarchy_y;
        float button_rotation = button_resolved_hierarchy_rotation;

        if(button_is_child_of_camera == false){
            float camera_resolved_hierarchy_x = 0.0f;
            float camera_resolved_hierarchy_y = 0.0f;
            float camera_resolved_hierarchy_rotation = 0.0f;
            node_base_get_child_absolute_xy(&camera_resolved_hierarchy_x, &camera_resolved_hierarchy_y, &camera_resolved_hierarchy_rotation, NULL, camera_node);
            camera_resolved_hierarchy_rotation = -camera_resolved_hierarchy_rotation;

            button_rotated_x = (button_rotated_x - camera_resolved_hierarchy_x) * camera_zoom;
            button_rotated_y = (button_rotated_y - camera_resolved_hierarchy_y) * camera_zoom;

            // Rotate rectangle origin about the camera
            engine_math_rotate_point(&button_rotated_x, &button_rotated_y, 0, 0, camera_resolved_hierarchy_rotation);

            button_rotation += camera_resolved_hierarchy_rotation;
        }else{
            camera_zoom = 1.0f;
        }

        button_rotated_x += camera_viewport->width/2;
        button_rotated_y += camera_viewport->height/2;

        font_resource_class_obj_t *font = button->font_resource;
        vector2_class_obj_t *button_scale = button->scale;

        float text_letter_spacing = mp_obj_get_float(button->letter_spacing);
        float text_line_spacing = mp_obj_get_float(button->line_spacing);

        float x_scale = button_scale->x.value*camera_zoom;
        float y_scale = button_scale->y.value*camera_zoom;

        // Decide which shader to use per-pixel
        engine_shader_t *shader = NULL;
        if(button_opacity < 1.0f){
            shader = engine_get_builtin_shader(OPACITY_SHADER);
        }else{
            shader = engine_get_builtin_shader(EMPTY_SHADER);
        }

        color_class_obj_t *outline_color = button->outline_color;
        color_class_obj_t *background_color = button->background_color;
        color_class_obj_t *text_color = button->text_color;

        if(button->pressed){
            if(button->pressed_text_color != mp_const_none) text_color = button->pressed_text_color;
            if(button->pressed_outline_color != mp_const_none) outline_color = button->pressed_outline_color;
            if(button->pressed_background_color != mp_const_none) background_color = button->pressed_background_color;
        }else if(button->focused){
            if(button->focused_text_color != mp_const_none) text_color = button->focused_text_color;
            if(button->focused_outline_color != mp_const_none) outline_color = button->focused_outline_color;
            if(button->focused_background_color != mp_const_none) background_color = button->focused_background_color;
        }

        engine_draw_rect(outline_color->value,
                         floorf(button_rotated_x), floorf(button_rotated_y),
                         (int32_t)button->width_outline, (int32_t)button->height_outline,
                         x_scale, y_scale,
                        -button_rotation,
                         button_opacity,
                         shader);

        engine_draw_rect(background_color->value,
                         floorf(button_rotated_x), floorf(button_rotated_y),
                         (int32_t)button->width_padded, (int32_t)button->height_padded,
                         x_scale, y_scale,
                        -button_rotation,
                         button_opacity,
                         shader);


        engine_shader_t *text_shader = NULL;

        if(text_color == mp_const_none){
            text_shader = engine_get_builtin_shader(EMPTY_SHADER);
        }else{
            text_shader = engine_get_builtin_shader(BLEND_OPACITY_SHADER);

            float t = 1.0f;

            text_shader->program[1] = (text_color->value >> 8) & 0b11111111;
            text_shader->program[2] = (text_color->value >> 0) & 0b11111111;

            memcpy(text_shader->program+3, &t, sizeof(float));
        }

        engine_draw_text(font, button->text,
                         floorf(button_rotated_x), floorf(button_rotated_y),
                         button->width, button->height,
                         text_letter_spacing,
                         text_line_spacing,
                         x_scale, y_scale,
                         button_rotation,
                         button_opacity,
                         text_shader);
    }
}


void gui_button_2d_node_calculate_dimensions(engine_gui_button_2d_node_class_obj_t *button){
    if(button->text != mp_const_none && button->font_resource != mp_const_none){
        font_resource_get_box_dimensions(button->font_resource, button->text, &button->width, &button->height, mp_obj_get_float(button->letter_spacing), mp_obj_get_float(button->line_spacing));
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

    // If this node is focused but being deleted, tell the GUI engine
    if(gui_button->focused) engine_gui_clear_focused();

    engine_collections_untrack_gui(gui_button->gui_list_node);

    node_base_del(self_in);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(gui_button_2d_node_class_del_obj, gui_button_2d_node_class_del);


// Return `true` if handled loading the attr from internal structure, `false` otherwise
bool button_2d_node_load_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
    // Get the underlying structure
    engine_gui_button_2d_node_class_obj_t *self = self_node_base->node;

    switch(attribute){
        case MP_QSTR___del__:
            destination[0] = MP_OBJ_FROM_PTR(&gui_button_2d_node_class_del_obj);
            destination[1] = self_node_base;
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
            destination[0] = self->button ? MP_OBJ_FROM_PTR(self->button) : mp_const_none;
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
bool button_2d_node_store_attr(engine_node_base_t *self_node_base, qstr attribute, mp_obj_t *destination){
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
            self->text_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_focused_text_color:
            self->focused_text_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_pressed_text_color:
            self->pressed_text_color = engine_color_wrap(destination[1]);
            return true;
        break;

        case MP_QSTR_background_color:
            self->background_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_focused_background_color:
            self->focused_background_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_pressed_background_color:
            self->pressed_background_color = engine_color_wrap(destination[1]);
            return true;
        break;

        case MP_QSTR_outline_color:
            self->outline_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_focused_outline_color:
            self->focused_outline_color = engine_color_wrap(destination[1]);
            return true;
        break;
        case MP_QSTR_pressed_outline_color:
            self->pressed_outline_color = engine_color_wrap(destination[1]);
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
            if (!mp_obj_is_type(destination[1], &button_class_type)) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("GUIBitmapButton2DNode: 'button' must be a Button object!"));
            }
            self->button = MP_OBJ_TO_PTR(destination[1]);
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


/*  --- doc ---
    NAME: on_focused
    ID: on_focused
    DESC: Called every tick when the GUI button is focused
    PARAM:  [type=object]         [name=button]     [value=object (reference to the button that was pressed)]
    RETURN: None
*/
/*  --- doc ---
    NAME: on_just_focused
    ID: on_just_focused
    DESC: Called once when the GUI button is focused
    PARAM:  [type=object]         [name=button]     [value=object (reference to the button that was pressed)]
    RETURN: None
*/
/*  --- doc ---
    NAME: on_just_unfocused
    ID: on_just_unfocused
    DESC: Called once when the GUI button is unfocused
    PARAM:  [type=object]         [name=button]     [value=object (reference to the button that was pressed)]
    RETURN: None
*/
/*  --- doc ---
    NAME: on_pressed
    ID: on_pressed
    DESC: Called every tick once when the GUI button is being pressed
    PARAM:  [type=object]         [name=button]     [value=object (reference to the button that was pressed)]
    RETURN: None
*/
/*  --- doc ---
    NAME: on_just_pressed
    ID: on_just_pressed
    DESC: Called once once when the GUI button is pressed
    PARAM:  [type=object]         [name=button]     [value=object (reference to the button that was pressed)]
    RETURN: None
*/
/*  --- doc ---
    NAME: on_just_released
    ID: on_just_released
    DESC: Called once once when the GUI button is unpressed/released
    PARAM:  [type=object]         [name=button]     [value=object (reference to the button that was pressed)]
    RETURN: None
*/
static mp_attr_fun_t gui_button_2d_node_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing GUIButton2DNode attr");
    node_base_attr_handler(self_in, attribute, destination,
                          (attr_handler_func[]){button_2d_node_load_attr, node_base_load_attr},
                          (attr_handler_func[]){button_2d_node_store_attr, node_base_store_attr}, 2);
    return mp_const_none;
}


/*  --- doc ---
    NAME: GUIButton2DNode
    ID: GUIButton2DNode
    DESC: Draws a simple square button with an outline, background color, and text. The text, padding, and outline thicknesses define how large the button will be. There is no way to set the button to an exact size other than by hand tweaking the scale
    PARAM:  [type={ref_link:Vector2}]               [name=position]                                     [value={ref_link:Vector2}]
    PARAM:  [type={ref_link:FontResource}]          [name=font]                                         [value={ref_link:FontResource}]
    PARAM:  [type=string]                           [name=text]                                         [value=any]
    PARAM:  [type=float]                            [name=outline]                                      [value=any (how thick the outline should be, in px)]
    PARAM:  [type=float]                            [name=padding]                                      [value=any (amount of empty space between the text and outline, in px)]

    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=text_color]                                   [value=any color that the base text color should blend to (works best with white text)]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=focused_text_color]                           [value=any color]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=pressed_text_color]                           [value=any color]

    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=background_color]                             [value=any color]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=focused_background_color]                     [value=any color]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=pressed_background_color]                     [value=any color]

    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=outline_color]                                [value=any color]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=focused_outline_color]                        [value=any color]
    PARAM:  [type={ref_link:Color}|int (RGB565)]    [name=pressed_outline_color]                        [value=any color]

    PARAM:  [type=float]                            [name=rotation]                                     [value=any (radians)]
    PARAM:  [type={ref_link:Vector2}]               [name=scale]                                        [value={ref_link:Vector2}]
    PARAM:  [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]

    PARAM:  [type=float]                            [name=letter_spacing]                               [value=any]
    PARAM:  [type=float]                            [name=line_spacing]                                 [value=any]
    PARAM:  [type=int]                              [name=layer]                                        [value=0 ~ 127]


    ATTR:   [type=function]                         [name={ref_link:add_child}]                         [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_child}]                         [value=function]
    ATTR:   [type=function]                         [name={ref_link:get_child_count}]                   [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy}]            [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy_all}]        [value=function]
    ATTR:   [type=function]                         [name={ref_link:node_base_mark_destroy_children}]   [value=function]
    ATTR:   [type=function]                         [name={ref_link:remove_child}]                      [value=function]
    ATTR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
    ATTR:   [type=function]                         [name={ref_link:on_focused}]                        [value=function]
    ATTR:   [type=function]                         [name={ref_link:on_just_focused}]                   [value=function]
    ATTR:   [type=function]                         [name={ref_link:on_just_unfocused}]                 [value=function]
    ATTR:   [type=function]                         [name={ref_link:on_pressed}]                        [value=function]
    ATTR:   [type=function]                         [name={ref_link:on_just_pressed}]                   [value=function]
    ATTR:   [type=function]                         [name={ref_link:on_just_released}]                  [value=function]

    ATTR:   [type={ref_link:Vector2}]               [name=position]                                     [value={ref_link:Vector2}]
    ATTR:   [type={ref_link:FontResource}]          [name=font]                                         [value={ref_link:FontResource}]
    ATTR:   [type=string]                           [name=text]                                         [value=any]
    ATTR:   [type=float]                            [name=outline]                                      [value=any (how thick the outline should be, in px)]
    ATTR:   [type=float]                            [name=padding]                                      [value=any (amount of empty space between the text and outline, in px)]

    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=text_color]                                    [value=color that the base text color should blend to (works best with white text)]
    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=focused_text_color]                            [value=color]
    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=pressed_text_color]                            [value=color]

    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=background_color]                              [value=color]
    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=focused_background_color]                      [value=color]
    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=pressed_background_color]                      [value=color]

    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=outline_color]                                 [value=color]
    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=focused_outline_color]                         [value=color]
    ATTR:   [type={ref_link:Color}|int (RGB565)]   [name=pressed_outline_color]                         [value=color]

    ATTR:   [type=float]                            [name=rotation]                                     [value=any (radians)]
    ATTR:   [type={ref_link:Vector2}]               [name=scale]                                        [value={ref_link:Vector2}]
    ATTR:   [type=float]                            [name=opacity]                                      [value=0 ~ 1.0]

    ATTR:   [type=float]                            [name=letter_spacing]                               [value=any]
    ATTR:   [type=float]                            [name=line_spacing]                                 [value=any]

    ATTR:   [type=boolean]                          [name=focused]                                      [value=True or False (can be read to see if focused or set to focus it)]
    ATTR:   [type=boolean]                          [name=pressed]                                      [value=True or False (can be read to see if pressed or set to press it)]

    ATTR:   [type=float]                            [name=width]                                        [value=any (total width of the button, read-only)]
    ATTR:   [type=float]                            [name=height]                                       [value=any (total height of the button, read-only)]
    ATTR:   [type=int]                              [name=layer]                                        [value=0 ~ 127]

    OVRR:   [type=function]                         [name={ref_link:tick}]                              [value=function]
    OVRR:   [type=function]                         [name={ref_link:on_focused}]                        [value=function]
    OVRR:   [type=function]                         [name={ref_link:on_just_focused}]                   [value=function]
    OVRR:   [type=function]                         [name={ref_link:on_just_unfocused}]                 [value=function]
    OVRR:   [type=function]                         [name={ref_link:on_pressed}]                        [value=function]
    OVRR:   [type=function]                         [name={ref_link:on_just_pressed}]                   [value=function]
    OVRR:   [type=function]                         [name={ref_link:on_just_released}]                  [value=function]
*/
mp_obj_t gui_button_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New GUIButton2DNode");

    mp_arg_t allowed_args[] = {
        { MP_QSTR_child_class,              MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_position,                 MP_ARG_OBJ, {.u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(0.0f), mp_obj_new_float(0.0f)})} },
        { MP_QSTR_font,                     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_text,                     MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_outline,                  MP_ARG_OBJ, {.u_obj = mp_obj_new_float(2.0f)} },
        { MP_QSTR_padding,                  MP_ARG_OBJ, {.u_obj = mp_obj_new_float(2.0f)} },

        { MP_QSTR_text_color,               MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_focused_text_color,       MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_pressed_text_color,       MP_ARG_OBJ, {.u_obj = mp_const_none} },

        { MP_QSTR_background_color,         MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(0x7bef)} },
        { MP_QSTR_focused_background_color, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_pressed_background_color, MP_ARG_OBJ, {.u_obj = mp_const_none} },

        { MP_QSTR_outline_color,            MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(0xa554)} },
        { MP_QSTR_focused_outline_color,    MP_ARG_OBJ, {.u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xff40)})} },
        { MP_QSTR_pressed_outline_color,    MP_ARG_OBJ, {.u_obj = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xde60)})} },

        { MP_QSTR_rotation,                 MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0f)} },
        { MP_QSTR_scale,                    MP_ARG_OBJ, {.u_obj = vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(1.0f), mp_obj_new_float(1.0f)})} },
        { MP_QSTR_opacity,                  MP_ARG_OBJ, {.u_obj = mp_obj_new_float(1.0f)} },

        { MP_QSTR_letter_spacing,           MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0f)} },
        { MP_QSTR_line_spacing,             MP_ARG_OBJ, {.u_obj = mp_obj_new_float(0.0f)} },
        { MP_QSTR_layer,                    MP_ARG_INT, {.u_int = 0} }
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

                  rotation, scale, opacity,

                  letter_spacing,
                  line_spacing,
                  
                  layer};

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
    engine_node_base_t *node_base = mp_obj_malloc_with_finaliser(engine_node_base_t, &engine_gui_button_2d_node_class_type);
    node_base_init(node_base, &engine_gui_button_2d_node_class_type, NODE_TYPE_GUI_BUTTON_2D, parsed_args[layer].u_int);
    engine_gui_button_2d_node_class_obj_t *gui_button_2d_node = m_malloc(sizeof(engine_gui_button_2d_node_class_obj_t));
    node_base->node = gui_button_2d_node;
    node_base->attr_accessor = node_base;

    gui_button_2d_node->gui_list_node = engine_collections_track_gui(node_base);

    gui_button_2d_node->tick_cb = mp_const_none;
    gui_button_2d_node->on_focused_cb = mp_const_none;
    gui_button_2d_node->on_just_focused_cb = mp_const_none;
    gui_button_2d_node->on_just_unfocused_cb = mp_const_none;
    gui_button_2d_node->on_pressed_cb = mp_const_none;
    gui_button_2d_node->on_just_pressed_cb = mp_const_none;
    gui_button_2d_node->on_just_released_cb = mp_const_none;

    gui_button_2d_node->position = parsed_args[position].u_obj;
    gui_button_2d_node->font_resource = parsed_args[font].u_obj;
    gui_button_2d_node->text = parsed_args[text].u_obj;
    gui_button_2d_node->outline = parsed_args[outline].u_obj;
    gui_button_2d_node->padding = parsed_args[padding].u_obj;

    gui_button_2d_node->text_color = engine_color_wrap_opt(parsed_args[text_color].u_obj);
    gui_button_2d_node->focused_text_color = engine_color_wrap_opt(parsed_args[focused_text_color].u_obj);
    gui_button_2d_node->pressed_text_color = engine_color_wrap_opt(parsed_args[pressed_text_color].u_obj);

    gui_button_2d_node->background_color = engine_color_wrap(parsed_args[background_color].u_obj);
    gui_button_2d_node->focused_background_color = engine_color_wrap_opt(parsed_args[focused_background_color].u_obj);
    gui_button_2d_node->pressed_background_color = engine_color_wrap_opt(parsed_args[pressed_background_color].u_obj);

    gui_button_2d_node->outline_color = engine_color_wrap(parsed_args[outline_color].u_obj);
    gui_button_2d_node->focused_outline_color = engine_color_wrap_opt(parsed_args[focused_outline_color].u_obj);
    gui_button_2d_node->pressed_outline_color = engine_color_wrap_opt(parsed_args[pressed_outline_color].u_obj);

    gui_button_2d_node->rotation = parsed_args[rotation].u_obj;
    gui_button_2d_node->scale = parsed_args[scale].u_obj;
    gui_button_2d_node->opacity = parsed_args[opacity].u_obj;

    gui_button_2d_node->letter_spacing = parsed_args[letter_spacing].u_obj;
    gui_button_2d_node->line_spacing = parsed_args[line_spacing].u_obj;

    gui_button_2d_node->focused = false;
    gui_button_2d_node->pressed = false;
    gui_button_2d_node->last_pressed = false;
    gui_button_2d_node->button = &BUTTON_A;

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
            gui_button_2d_node->tick_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->tick_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_focused, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->on_focused_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->on_focused_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_focused, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->on_just_focused_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->on_just_focused_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_unfocused, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->on_just_unfocused_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->on_just_unfocused_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_pressed, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->on_pressed_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->on_pressed_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_pressed, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->on_just_pressed_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->on_just_pressed_cb = dest[0];
        }

        mp_load_method_maybe(node_instance, MP_QSTR_on_just_released, dest);
        if(dest[0] == MP_OBJ_NULL && dest[1] == MP_OBJ_NULL){   // Did not find method (set to default)
            gui_button_2d_node->on_just_released_cb = mp_const_none;
        }else{                                                  // Likely found method (could be attribute)
            gui_button_2d_node->on_just_released_cb = dest[0];
        }

        // Store one pointer on the instance. Need to be able to get the
        // node base that contains a pointer to the engine specific data we
        // care about
        // mp_store_attr(node_instance, MP_QSTR_node_base, node_base);
        mp_store_attr(node_instance, MP_QSTR_node_base, node_base);

        // Store default Python class instance attr function
        // and override with custom intercept attr function
        // so that certain callbacks/code can run (see py/objtype.c:mp_obj_instance_attr(...))
        node_base_set_attr_handler(node_instance, gui_button_2d_node_class_attr);

        // Need a way to access the object node instance instead of the native type for callbacks (tick, draw, collision)
        node_base->attr_accessor = node_instance;
    }

    gui_button_2d_node_calculate_dimensions(gui_button_2d_node);

    return MP_OBJ_FROM_PTR(node_base);
}


// Class attributes
static const mp_rom_map_elem_t gui_button_2d_node_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(gui_button_2d_node_class_locals_dict, gui_button_2d_node_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    engine_gui_button_2d_node_class_type,
    MP_QSTR_GUIButton2DNode,
    MP_TYPE_FLAG_NONE,

    make_new, gui_button_2d_node_class_new,
    attr, gui_button_2d_node_class_attr,
    locals_dict, &gui_button_2d_node_class_locals_dict
);