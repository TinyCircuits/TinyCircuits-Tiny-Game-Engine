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

extern const mp_obj_type_t const_color_class_type;
extern const mp_obj_type_t color_class_type;

float engine_color_get_r_float(uint16_t color);
float engine_color_get_g_float(uint16_t color);
float engine_color_get_b_float(uint16_t color);

uint16_t engine_color_set_r_float(uint16_t color, float r);
uint16_t engine_color_set_g_float(uint16_t color, float g);
uint16_t engine_color_set_b_float(uint16_t color, float b);

uint16_t engine_color_from_rgb_float(float r, float g, float b);

bool engine_color_is_instance(mp_obj_t obj);
bool engine_color_is_class(const mp_obj_type_t *type);

// Returns an RGB565 color value from the specified Color or int (RGB565).
uint16_t engine_color_class_color_value(mp_obj_t color);

// If the argument is a Color or ConstColor, returns itself. If the argument is an int, creates a Color from it.
// Otherwise fails.
mp_obj_t engine_color_wrap(mp_obj_t color);

// Same as engine_color_wrap, but also accepts None and returns None.
mp_obj_t engine_color_wrap_opt(mp_obj_t color_or_none);

mp_obj_t color_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

uint16_t ENGINE_FAST_FUNCTION(engine_color_blend)(uint16_t from, uint16_t to, float amount);
uint16_t ENGINE_FAST_FUNCTION(engine_color_alpha_blend)(uint16_t background, uint16_t foreground, float alpha);

#endif  /// ENGINE_COLOR_H
