#ifndef TEXT_2D_NODE_H
#define TEXT_2D_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// A basic 2d text node
typedef struct{
    mp_obj_t position;      // Vector2: 2d xy position of this node
    mp_obj_t font_resource; // FontResource
    mp_obj_t text;          // string: The text to display
    mp_obj_t rotation;      // Rotation about into screen/z-axis in degrees
    mp_obj_t scale;         // Vector2
    mp_obj_t opacity;
    mp_obj_t letter_spacing;
    mp_obj_t line_spacing;
    mp_obj_t width;         // Width, in int pixels, of the box containing the text
    mp_obj_t height;        // height, in int pixels, of the box containing the text
    mp_obj_t color;
    mp_obj_t tick_cb;
}engine_text_2d_node_class_obj_t;

extern const mp_obj_type_t engine_text_2d_node_class_type;
void text_2d_node_class_draw(engine_node_base_t *text_node_base, mp_obj_t camera_node);
mp_obj_t text_2d_node_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // TEXT_2D_NODE_H