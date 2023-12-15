#ifndef RECTANGLE_2D_NODE_H
#define RECTANGLE_2D_NODE_H

#include "py/obj.h"

// A polygon 2d node
typedef struct{
    mp_obj_t position;  // Vector2: 2d xy position of this node
    mp_obj_t color;     // The color of this rectangle
    mp_obj_t rotation;  // Rectangle rotation in radians
}engine_rectangle_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
}engine_rectangle_2d_node_common_data_t;

extern const mp_obj_type_t engine_rectangle_2d_node_class_type;


#endif  // RECTANGLE_2D_NODE_H
