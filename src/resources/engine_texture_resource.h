#ifndef ENGINE_TEXTURE_RESOURCE_H
#define ENGINE_TEXTURE_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_file.h"

typedef struct texture_resource_class_obj_t{
    mp_obj_base_t base;
    int32_t width;
    int32_t height;
    uint8_t bit_depth;

    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
    uint32_t combined_masks;

    mp_obj_t colors;
    mp_obj_t data;
    bool in_ram;

    // Used by 16 xrgb or argb formats to move masked bits
    // all the way to the right for mapping
    uint16_t a_mask_right_shift_amount;
    uint16_t r_mask_right_shift_amount;
    uint16_t g_mask_right_shift_amount;
    uint16_t b_mask_left_shift_amount;

    // Custom assigned function for getting pixels
    // from the texture_resource instance at an offset
    uint16_t (*get_pixel)(struct texture_resource_class_obj_t *texture, uint32_t offset, float *out_alpha);
}texture_resource_class_obj_t;

extern const mp_obj_type_t texture_resource_class_type;


uint16_t texture_resource_get_indexed_pixel(texture_resource_class_obj_t *texture, uint32_t pixel_offset, float *out_alpha);
uint16_t texture_resource_get_16bit_rgb565(texture_resource_class_obj_t *texture, uint32_t pixel_offset, float *out_alpha);
uint16_t texture_resource_get_16bit_axrgb(texture_resource_class_obj_t *texture, uint32_t pixel_offset, float *out_alpha);
mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_TEXTURE_RESOURCE_H