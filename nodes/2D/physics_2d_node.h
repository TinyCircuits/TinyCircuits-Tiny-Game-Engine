#ifndef PHYSICS_2D_NODE_H
#define PHYSICS_2D_NODE_H

#include "py/obj.h"

typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t velocity;              // Vector2
    mp_obj_t acceleration;          // Vector2 
}engine_physics_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
}engine_physics_2d_node_common_data_t;

extern const mp_obj_type_t engine_physics_2d_node_class_type;

#endif  // PHYSICS_2D_NODE_H