#ifndef RECTANGLE_2D_NODE_H
#define RECTANGLE_2D_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "utility/linked_list.h"

// A basic 2d rectangle node
typedef struct{
    mp_obj_t position;  // Vector2: 2d xy position of this node
    mp_obj_t width;     // Rectangle width in px
    mp_obj_t height;    // Rectangle height in px
    mp_obj_t color;     // The color of this rectangle
}engine_rectangle_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
}engine_rectangle_2d_node_common_data_t;

extern const mp_obj_type_t engine_rectangle_2d_node_class_type;


#endif  // RECTANGLE_2D_NODE_H