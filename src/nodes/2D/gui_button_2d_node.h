#ifndef GUI_BUTTON_2D_NODE_H
#define GUI_BUTTON_2D_NODE_H

#include "py/obj.h"
#include "math/vector2.h"
#include "nodes/node_base.h"
#include "utility/linked_list.h"

typedef struct{
    mp_obj_t position;
    mp_obj_t font_resource;
    mp_obj_t text;
    mp_obj_t outline;
    mp_obj_t padding;

    mp_obj_t text_color;
    mp_obj_t focused_text_color;
    mp_obj_t pressed_text_color;

    mp_obj_t background_color;
    mp_obj_t focused_background_color;
    mp_obj_t pressed_background_color;

    mp_obj_t outline_color;
    mp_obj_t focused_outline_color;
    mp_obj_t pressed_outline_color;

    mp_obj_t rotation;
    mp_obj_t scale;
    mp_obj_t opacity;

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
    uint16_t button;

    float width;
    float height;
    float width_padded;
    float height_padded;
    float width_outline;
    float height_outline;

    linked_list_node *gui_list_node;
}engine_gui_button_2d_node_class_obj_t;

extern const mp_obj_type_t engine_gui_button_2d_node_class_type;
void gui_button_2d_node_class_draw(engine_node_base_t *button_node_base, mp_obj_t camera_node);

#endif  // GUI_BUTTON_2D_NODE_H