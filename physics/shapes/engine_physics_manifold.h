#ifndef ENGINE_PHYSICS_MANIFOLD_H
#define ENGINE_PHYSICS_MANIFOLD_H

#include "py/obj.h"
#include <math.h>

typedef struct physics_manifold_class_obj_t {
    mp_obj_base_t base;
    mp_float_t mtv_x;
    mp_float_t mtv_y;
    mp_float_t con_x;
    mp_float_t con_y;
    mp_float_t nrm_x;
    mp_float_t nrm_y;
} physics_manifold_class_obj_t;

extern const mp_obj_type_t physics_manifold_class_type;

mp_obj_t physics_manifold_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

// Default no-overlap manifold
const STATIC physics_manifold_class_obj_t const_separated_manifold = {{&physics_manifold_class_type}, (mp_float_t)0.0, (mp_float_t)0.0, (mp_float_t)NAN, (mp_float_t)NAN, (mp_float_t)0.0, (mp_float_t)0.0};

#endif // ENGINE_PHYSICS_MANIFOLD_H
