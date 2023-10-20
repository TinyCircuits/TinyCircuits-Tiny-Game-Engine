#ifndef VECTOR3_H
#define VECTOR3_H

#include "py/obj.h"

typedef struct{
    mp_obj_base_t base;
    mp_float_t x;
    mp_float_t y;
    mp_float_t z;
}vector3_class_obj_t;

const mp_obj_type_t vector3_class_type;

mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // VECTOR3_H