#ifndef VECTOR2_H
#define VECTOR2_H

#include "py/obj.h"
#include <math.h>

typedef struct{
    mp_obj_base_t base;
    mp_obj_t x;
    mp_obj_t y;
}vector2_class_obj_t;

extern const mp_obj_type_t vector2_class_type;

mp_obj_t vector2_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // VECTOR2_H
