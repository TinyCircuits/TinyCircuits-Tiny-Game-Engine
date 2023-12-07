#ifndef ENGINE_PHYSICS_SHAPE_CONVEX_H
#define ENGINE_PHYSICS_SHAPE_CONVEX_H

#include "py/obj.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_t v_list;
    mp_obj_t n_list;
    mp_float_t area;
    mp_float_t I;
    mp_obj_t center;
}physics_shape_convex_class_obj_t;

extern const mp_obj_type_t physics_shape_convex_class_type;

STATIC mp_obj_t physics_shape_convex_class_compute_normals(mp_obj_t self_in); // Recompute polygon edge/surface normals
STATIC mp_obj_t physics_shape_convex_class_compute_all(mp_obj_t self_in); // Recompute area, geocenter, and inertia tensor

mp_obj_t physics_shape_convex_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_PHYSICS_SHAPE_CONVEX_H
