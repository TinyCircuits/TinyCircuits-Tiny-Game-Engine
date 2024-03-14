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
    mp_obj_t width;         // Width, in int pixels, of the box containing the text
    mp_obj_t height;        // height, in int pixels, of the box containing the text
}engine_text_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
}engine_text_2d_node_common_data_t;

extern const mp_obj_type_t engine_text_2d_node_class_type;
void text_2d_node_class_draw(engine_node_base_t *text_node_base, mp_obj_t camera_node);

#endif  // TEXT_2D_NODE_H