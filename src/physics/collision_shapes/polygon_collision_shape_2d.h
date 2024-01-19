#ifndef POLYGON_COLLISION_SHAPE_2D_H
#define POLYGON_COLLISION_SHAPE_2D_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "math/vector2.h"

typedef struct{
    mp_obj_base_t base;
    vector2_class_obj_t *position;
    mp_obj_list_t *vertices;
    mp_obj_list_t *normals;   // List of normals
}polygon_collision_shape_2d_class_obj_t;

extern const mp_obj_type_t polygon_collision_shape_2d_class_type;

#endif  // POLYGON_COLLISION_SHAPE_2D_H
