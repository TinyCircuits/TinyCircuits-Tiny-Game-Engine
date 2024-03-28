#ifndef CIRCLE_2D_NODE_H
#define CIRCLE_2D_NODE_H

#include "py/obj.h"
#include "nodes/node_base.h"


// A basic 2d circle node
typedef struct{
    mp_obj_t position;  // Vector2: 2d xy position of this node
    mp_obj_t radius;    // float
    mp_obj_t rotation;  // float Rotation in radians (affects child nodes)
    mp_obj_t color;     // int The color of this circle
    mp_obj_t opacity;
    mp_obj_t scale;     // float: how much to scale radius by, 1.0f by default
    mp_obj_t outline;   // bool: if true, circle is drawn as an outline, false by default
}engine_circle_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
}engine_circle_2d_node_common_data_t;

extern const mp_obj_type_t engine_circle_2d_node_class_type;
void circle_2d_node_class_draw(engine_node_base_t *circle_node_base, mp_obj_t camera_node);


#endif  // CIRCLE_2D_NODE_H
