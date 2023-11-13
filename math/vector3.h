#ifndef VECTOR3_H
#define VECTOR3_H

#include "py/obj.h"
#include <math.h>

typedef struct{
    mp_obj_base_t base;
    mp_obj_t x;
    mp_obj_t  y;
    mp_obj_t  z;
}vector3_class_obj_t;

extern const mp_obj_type_t vector3_class_type;

mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // VECTOR3_H
