#ifndef COLLISION_CONTACT_2D_H
#define COLLISION_CONTACT_2D_H

#include "py/obj.h"
#include "utility/linked_list.h"
#include "nodes/node_base.h"
#include "math/vector2.h"

typedef struct{
    mp_obj_base_t base;
    vector2_class_obj_t *position;
    vector2_class_obj_t *normal;
    mp_obj_t node;                  // The other node
}collision_contact_2d_class_obj_t;

extern const mp_obj_type_t collision_contact_2d_class_type;

mp_obj_t collision_contact_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // COLLISION_CONTACT_2D_H
