#ifndef ENGINE_PHYSICS_SHAPE_CIRCLE_H
#define ENGINE_PHYSICS_SHAPE_CIRCLE_H

#include "py/obj.h"

typedef struct{
    mp_obj_base_t base;
    mp_float_t radius;
    void *physics_shape_circle;
}physics_shape_circle_class_obj_t;

extern const mp_obj_type_t physics_shape_circle_class_type;

mp_obj_t physics_shape_circle_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_PHYSICS_SHAPE_CIRCLE_H
