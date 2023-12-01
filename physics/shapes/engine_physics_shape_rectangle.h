#ifndef ENGINE_PHYSICS_SHAPE_RECTANGLE_H
#define ENGINE_PHYSICS_SHAPE_RECTANGLE_H

#include "py/obj.h"

typedef struct{
    mp_obj_base_t base;
    mp_float_t width;
    mp_float_t height;
    void *physac_shape;
}physics_shape_rectangle_class_obj_t;

extern const mp_obj_type_t physics_shape_rectangle_class_type;

mp_obj_t physics_shape_rectangle_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_PHYSICS_SHAPE_RECTANGLE_H