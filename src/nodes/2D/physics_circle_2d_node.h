#ifndef PHYSICS_CIRCLE_2D_NODE_H
#define PHYSICS_CIRCLE_2D_NODE_H


#include "py/obj.h"
#include "nodes/node_base.h"
#include "utility/linked_list.h"


typedef struct{
    mp_obj_t radius;                        // Radius of the collider
}engine_physics_circle_2d_node_class_obj_t;


extern const mp_obj_type_t engine_physics_circle_2d_node_class_type;

// Call this after changing the rotation of the physics
// node so that the shape (if needed) can be updated
void engine_physics_circle_2d_node_update(engine_physics_circle_2d_node_class_obj_t *self);

#endif  // PHYSICS_CIRCLE_2D_NODE_H