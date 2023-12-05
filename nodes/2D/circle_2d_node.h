#ifndef CIRCLE_2D_NODE_H
#define CIRCLE_2D_NODE_H

#include "py/obj.h"

// A basic 2d circle node
typedef struct{
    mp_obj_t position;  // Vector2: 2d xy position of this node
    mp_obj_t radius;    // mp_float_t
    mp_obj_t rotation;  // mp_float_t Rotation in radians (affects child nodes)
    mp_obj_t color;     // int The color of this circle
}engine_circle_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
}engine_circle_2d_node_common_data_t;

extern const mp_obj_type_t engine_circle_2d_node_class_type;


#endif  // CIRCLE_2D_NODE_H
