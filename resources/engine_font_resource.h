#ifndef ENGINE_FONT_RESOURCE_H
#define ENGINE_FONT_RESOURCE_H

#include "py/obj.h"

typedef struct{
    mp_obj_base_t base;
    unsigned char* filename;
    mp_float_t font_size;
}font_resource_class_obj_t;

extern const mp_obj_type_t font_resource_class_type;

mp_obj_t font_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_FONT_RESOURCE_H