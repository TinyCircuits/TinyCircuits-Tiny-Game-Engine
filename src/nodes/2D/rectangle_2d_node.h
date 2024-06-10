#ifndef RECTANGLE_2D_NODE_H
#define RECTANGLE_2D_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"

// A basic 2d rectangle node
typedef struct{
    mp_obj_t position;  // Vector2: 2d xy position of this node
    mp_obj_t width;     // Rectangle width in px
    mp_obj_t height;    // Rectangle height in px
    mp_obj_t color;     // The color of this rectangle
    mp_obj_t opacity;
    mp_obj_t outline;   // bool: if true, rectangle drawn as outline, if false, drawn filled (false by default)
    mp_obj_t rotation;  // Rectangle rotation in radians
    mp_obj_t scale;     // Vector2: 2d scale of the rectangle
    mp_obj_t tick_cb;
}engine_rectangle_2d_node_class_obj_t;

extern const mp_obj_type_t engine_rectangle_2d_node_class_type;
void rectangle_2d_node_class_draw(mp_obj_t rectangle_node_base_obj, mp_obj_t camera_node);


#endif  // RECTANGLE_2D_NODE_H
