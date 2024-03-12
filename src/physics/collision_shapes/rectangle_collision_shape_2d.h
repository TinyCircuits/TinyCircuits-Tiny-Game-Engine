#ifndef RECTANGLE_COLLISION_SHAPE_2D_H
#define RECTANGLE_COLLISION_SHAPE_2D_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "math/vector2.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_t width;
    mp_obj_t height;

    float rotation; // Set by `Physics2DNode` when its rotation changes

    float vertices_x[4];
    float vertices_y[4];

    float normals_x[4];
    float normals_y[4];
}rectangle_collision_shape_2d_class_obj_t;

extern const mp_obj_type_t rectangle_collision_shape_2d_class_type;

void rectangle_collision_shape_2d_recalculate(mp_obj_t self_in);

#endif  // RECTANGLE_COLLISION_SHAPE_2D_H
