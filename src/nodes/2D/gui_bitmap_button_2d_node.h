#ifndef GUI_BITMAP_BUTTON_2D_NODE_H
#define GUI_BITMAP_BUTTON_2D_NODE_H

#include "py/obj.h"
#include "math/vector2.h"
#include "nodes/node_base.h"
#include "utility/linked_list.h"
#include "io/engine_io_buttons.h"

typedef struct{
    mp_obj_t position;
    mp_obj_t font_resource;
    mp_obj_t text;

    mp_obj_t text_color;
    mp_obj_t focused_text_color;
    mp_obj_t pressed_text_color;

    mp_obj_t bitmap_texture;
    mp_obj_t focused_bitmap_texture;
    mp_obj_t pressed_bitmap_texture;

    mp_obj_t transparent_color;

    mp_obj_t rotation;
    mp_obj_t scale;
    mp_obj_t text_scale;
    mp_obj_t opacity;

    mp_obj_t letter_spacing;
    mp_obj_t line_spacing;
    mp_obj_t disabled;

    mp_obj_t tick_cb;
    mp_obj_t on_focused_cb;
    mp_obj_t on_just_focused_cb;
    mp_obj_t on_just_unfocused_cb;
    mp_obj_t on_pressed_cb;
    mp_obj_t on_just_pressed_cb;
    mp_obj_t on_just_released_cb;

    bool focused;
    bool last_focused;
    bool pressed;
    bool last_pressed;
    button_class_obj_t* button;

    float text_width;
    float text_height;

    linked_list_node *gui_list_node;
}engine_gui_bitmap_button_2d_node_class_obj_t;

extern const mp_obj_type_t engine_gui_bitmap_button_2d_node_class_type;
void gui_bitmap_button_2d_node_class_draw(mp_obj_t button_node_base_obj, mp_obj_t camera_node);

#endif  // GUI_BITMAP_BUTTON_2D_NODE_H