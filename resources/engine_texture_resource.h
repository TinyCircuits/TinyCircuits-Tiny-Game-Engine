#ifndef ENGINE_TEXTURE_RESOURCE_H
#define ENGINE_TEXTURE_RESOURCE_H

#include "py/obj.h"
#include "libs/mpfile/mpfile.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_t width;
    mp_obj_t height;
    uint32_t *block_address;
}texture_resource_class_obj_t;

extern const mp_obj_type_t texture_resource_class_type;

mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
mp_obj_t texture_resource_get_pixel(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in);

void erase_remaining_flash_resource_space();

#endif  // ENGINE_TEXTURE_RESOURCE_H