#ifndef PHYSICS_RECTANGLE_2D_NODE_H
#define PHYSICS_RECTANGLE_2D_NODE_H


#include "py/obj.h"
#include "nodes/node_base.h"
#include "utility/linked_list.h"


typedef struct{
    mp_obj_t position;              // Vector2: 2d xy position of this node
    mp_obj_t width;                 // Width of the collider
    mp_obj_t height;                // Height of the collider
    mp_obj_t velocity;              // Vector2 (Absolute velocity)
    mp_obj_t acceleration;          // Vector2
    mp_obj_t rotation;              // float (Current rotation angle)
    mp_obj_t mass;                  // How heavy the node is
    mp_obj_t bounciness;            // Restitution or elasticity

    mp_obj_t dynamic;               // Flag indicating if node is dynamic and moving around due to physics or static
    mp_obj_t solid;                 // May want collision callbacks to happen without impeding objects, set to false

    mp_obj_t gravity_scale;         // Vector2 allowing scaling affects of gravity. Set to 0,0 for no gravity

    uint8_t physics_id;

    float vertices_x[4];
    float vertices_y[4];

    float normals_x[2];
    float normals_y[2];

    mp_obj_t tick_cb;
    mp_obj_t collision_cb;
    linked_list_node *physics_list_node;    // All physics 2d nodes get added to a list that is easy to traverse
}engine_physics_rectangle_2d_node_class_obj_t;


extern const mp_obj_type_t engine_physics_rectangle_2d_node_class_type;

// Call this after changing the rotation of the physics
// node so that the shape (if needed) can be updated
void engine_physics_rectangle_2d_node_update(engine_physics_rectangle_2d_node_class_obj_t *self);

#endif  // PHYSICS_RECTANGLE_2D_NODE_H