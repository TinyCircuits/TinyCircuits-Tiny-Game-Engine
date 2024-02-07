#ifndef PHYSICS_2D_NODE_H
#define PHYSICS_2D_NODE_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"


typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t collision_shape;       // Shape definition
    mp_obj_t velocity;              // Vector2 (Absolute velocity)
    mp_obj_t acceleration;          // Vector2
    mp_obj_t rotation;              // float (Current rotation angle)
    mp_obj_t mass;                  // How heavy the node is
    mp_obj_t bounciness;            // Restitution or elasticity
    mp_obj_t dynamic;               // Flag indicating if node is dynamic and moving around due to physics or static
    mp_obj_t gravity_scale;         // Vector2 allowing scaling affects of gravity. Set to 0,0 for no gravity
    // mp_obj_t angular_velocity;    // float (Angular velocity)
    // mp_obj_t i_mass;              // float (inverse mass)
    // mp_obj_t i_I;            // float (inverse inertia tensor)
    // mp_obj_t restitution;         // float (restitution coefficient)
    // mp_obj_t friction;         // float (simple friction coefficient)
}engine_physics_2d_node_class_obj_t;

typedef struct{
    mp_obj_t tick_cb;
    mp_obj_t draw_cb;
    mp_obj_t collision_cb;
    float penetration;
    linked_list_node *physics_list_node;    // All physics 2d nodes get added to a list that is easy to traverse
}engine_physics_2d_node_common_data_t;

extern const mp_obj_type_t engine_physics_2d_node_class_type;

mp_obj_t physics_2d_node_class_apply_manifold_impulse(mp_obj_t a_in, mp_obj_t b_in, mp_obj_t manifold_in);
mp_obj_t physics_2d_node_class_test(mp_obj_t self_in, mp_obj_t b_in);

#endif  // PHYSICS_2D_NODE_H
