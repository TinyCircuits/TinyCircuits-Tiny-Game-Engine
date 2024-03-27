#ifndef ENGINE_COLOR_H
#define ENGINE_COLOR_H

#include "py/obj.h"
#include "py/objint.h"
#include <math.h>
#include "utility/engine_mp.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_float_t r;
    mp_obj_float_t g;
    mp_obj_float_t b;
    mp_obj_int_t value;
}color_class_obj_t;

extern const mp_obj_type_t color_class_type;

mp_obj_t color_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

void engine_color_sync_u16_to_rgb(color_class_obj_t *color);
void engine_color_sync_rgb_to_u16(color_class_obj_t *color);

#endif  /// ENGINE_COLOR_H