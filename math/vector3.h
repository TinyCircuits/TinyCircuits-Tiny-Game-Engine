#ifndef VECTOR3_H
#define VECTOR3_H

#include "py/obj.h"
#include <math.h>

typedef struct{
    mp_obj_base_t base;
    mp_float_t x;
    mp_float_t y;
    mp_float_t z;
}vector3_class_obj_t;

extern const mp_obj_type_t vector3_class_type;

mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

// STATIC mp_obj_t vector3_class_dot(mp_obj_t _self, mp_obj_t _b);
// STATIC mp_obj_t vector3_class_cross(mp_obj_t _self, mp_obj_t _b);
// STATIC mp_obj_t vector3_class_len2(mp_obj_t _self);
// STATIC mp_obj_t vector3_class_len(mp_obj_t _self);
// STATIC mp_obj_t vector3_class_normal(mp_obj_t _self);
// STATIC mp_obj_t vector3_class_normalize(mp_obj_t _self);
// STATIC mp_obj_t vector3_class_rotateZ(mp_obj_t _self, mp_obj_t _theta;
// STATIC mp_obj_t vector3_class_rotateY(mp_obj_t _self, mp_obj_t _theta);
// STATIC mp_obj_t vector3_class_rotateX(mp_obj_t _self, mp_obj_t _theta);
// STATIC mp_obj_t vector3_class_rotate(mp_obj_t _self, mp_obj_t _axis, mp_obj_t _theta);
// STATIC mp_obj_t vector3_class_resize(mp_obj_t _self, mp_obj_t _b);

#endif  // VECTOR3_H
