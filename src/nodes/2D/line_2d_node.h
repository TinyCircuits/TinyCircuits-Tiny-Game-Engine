#ifndef LINE_2D_NODE_H
#define LINE_2D_NODE_H

#include "py/obj.h"
#include "math/vector2.h"
#include "nodes/node_base.h"

// A basic 2d line node
typedef struct{
    mp_obj_t start;     // Vector2: start 2d xy position of this node
    mp_obj_t end;       // Vector2: end 2d xy position of this node
    mp_obj_t position;  // Vector2: midpoint between start and end
    mp_obj_t thickness; // How thick the line should be, in pixels (ceil to clamp up)
    mp_obj_t color;     // int The color of this line
    mp_obj_t opacity;
    mp_obj_t outline;   // bool: if true, line is drawn as an outline, false by default
    mp_obj_t tick_cb;
}engine_line_2d_node_class_obj_t;

extern const mp_obj_type_t engine_line_2d_node_class_type;
void line_2d_node_class_draw(mp_obj_t line_node_base_obj, mp_obj_t camera_node);

#endif  // LINE_2D_NODE_H
