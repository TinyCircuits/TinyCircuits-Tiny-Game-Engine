#ifndef ENGINE_TEXTURE_RESOURCE_H
#define ENGINE_TEXTURE_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_file.h"

typedef struct{
    mp_obj_base_t base;
    uint16_t width;
    uint16_t height;
    uint8_t bit_depth;
    uint8_t red_mask;
    uint8_t green_mask;
    uint8_t blue_mask;
    uint8_t alpha_mask;
    mp_obj_t colors;
    mp_obj_t data;
    bool in_ram;
}texture_resource_class_obj_t;

extern const mp_obj_type_t texture_resource_class_type;

mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
uint16_t texture_resource_get_pixel(texture_resource_class_obj_t *texture, uint32_t offset);

#endif  // ENGINE_TEXTURE_RESOURCE_H