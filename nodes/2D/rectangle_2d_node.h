#ifndef RECTANGLE_2D_NODE_H
#define RECTANGLE_2d_NODE_H

#include "py/obj.h"
#include "../node_base.h"
#include "math/vector3.h"
#include "math/rectangle.h"
#include "utility/linked_list.h"

// A basic 2d rectangle node
typedef struct{
    mp_obj_base_t base;             // MicroPython base (also used in engine for checking the type of generic node. This must be the first element of any node)
    engine_node_base_t node_base;   // Engine node base (holds information about type, linked list location, draw/execution layer, visible, disabled, or just added. Required, must be second element)
    mp_obj_t tick_dest[2];          // Used for caching data used for calling the 'tick()' callback on instances of this node
    mp_obj_t draw_dest[2];          // Used for caching data used for calling the 'draw()' callback on instances of this node
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t width;                 // Rectangle width in px
    mp_obj_t height;                // Rectangle height in px
    mp_obj_t color;                 // The color of this rectangle
}engine_rectangle_2d_node_class_obj_t;

extern const mp_obj_type_t engine_rectangle_2d_node_class_type;


#endif  // RECTANGLE_2d_NODE_H