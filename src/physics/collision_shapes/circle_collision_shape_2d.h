#ifndef CIRCLE_COLLISION_SHAPE_2D_H
#define CIRCLE_COLLISION_SHAPE_2D_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "math/vector2.h"

typedef struct{
    mp_obj_base_t base;
    vector2_class_obj_t *position;              // Vector2: 2d xy position of this node
    float radius;
}circle_collision_shape_2d_class_obj_t;

extern const mp_obj_type_t circle_collision_shape_2d_class_type;

#endif  // CIRCLE_COLLISION_SHAPE_2D_H
