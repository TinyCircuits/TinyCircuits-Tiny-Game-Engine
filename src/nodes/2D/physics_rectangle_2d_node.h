#ifndef PHYSICS_RECTANGLE_2D_NODE_H
#define PHYSICS_RECTANGLE_2D_NODE_H


#include "py/obj.h"
#include "nodes/node_base.h"
#include "nodes/physics_node_base.h"
#include "utility/linked_list.h"


typedef struct{
    mp_obj_t width;                 // Width of the collider
    mp_obj_t height;                // Height of the collider

    float vertices_x[4];
    float vertices_y[4];

    float normals_x[2];
    float normals_y[2];
}engine_physics_rectangle_2d_node_class_obj_t;


extern const mp_obj_type_t engine_physics_rectangle_2d_node_class_type;

// Call this after changing the rotation of the physics
// node so that the shape (if needed) can be updated
void engine_physics_rectangle_2d_node_update(engine_physics_node_base_t *physics_node_base);

void physics_rectangle_2d_node_class_draw(mp_obj_t rectangle_node_base_obj, mp_obj_t camera_node);

#endif  // PHYSICS_RECTANGLE_2D_NODE_H