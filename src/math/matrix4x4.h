#ifndef MATRIX4x4_H
#define MATRIX4x4_H

#include "py/obj.h"
#include <math.h>
#include "utility/engine_mp.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_float_t m[4][4];
}matrix4x4_class_obj_t;

extern const mp_obj_type_t matrix4x4_class_type;

mp_obj_t matrix4x4_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // MATRIX4x4_H