#ifndef ENGINE_COLOR_H
#define ENGINE_COLOR_H

#include "py/obj.h"
#include "py/objint.h"
#include <math.h>
#include "utility/engine_mp.h"
#include "utility/engine_defines.h"

typedef struct{
    mp_obj_base_t base;
    uint16_t value;
}color_class_obj_t;

extern const mp_obj_type_t color_class_type;

float engine_color_get_r_float(uint16_t color);
float engine_color_get_g_float(uint16_t color);
float engine_color_get_b_float(uint16_t color);

uint16_t engine_color_set_r_float(uint16_t color, float r);
uint16_t engine_color_set_g_float(uint16_t color, float g);
uint16_t engine_color_set_b_float(uint16_t color, float b);

uint16_t engine_color_from_rgb_float(float r, float g, float b);

mp_obj_t color_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

uint16_t ENGINE_FAST_FUNCTION(engine_color_blend)(uint16_t from, uint16_t to, float amount);
uint16_t ENGINE_FAST_FUNCTION(engine_color_alpha_blend)(uint16_t background, uint16_t foreground, float alpha);

#endif  /// ENGINE_COLOR_H
