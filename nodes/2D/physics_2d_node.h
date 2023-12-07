#ifndef PHYSICS_2D_NODE_H
#define PHYSICS_2D_NODE_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"


typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t rotation;              // float (Current rotation angle)
    mp_obj_t velocity;              // Vector2 (Absolute velocity)
    mp_obj_t acceleration;          // Vector2 (Reference acceleration)
    mp_float_t angular_velocity;    // float (Angular velocity)
    mp_float_t i_mass;              // float (inverse mass)
    mp_float_t I_tensor;            // float (inertia tensor)
    mp_obj_t dynamic;               // bool
    mp_obj_t physics_shape;         // Shape definition
}engine_physics_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
    void *physics_body;
    linked_list_node *physics_list_node;    // All physics 2d nodes get added to a list that is easy to traverse
}engine_physics_2d_node_common_data_t;

extern const mp_obj_type_t engine_physics_2d_node_class_type;

#endif  // PHYSICS_2D_NODE_H
