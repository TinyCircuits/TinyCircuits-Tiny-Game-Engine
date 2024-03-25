#ifndef VECTOR2_H
#define VECTOR2_H

#include "py/obj.h"
#include <math.h>
#include "utility/engine_mp.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_float_t x;
    mp_obj_float_t y;
    void *on_change_user_ptr;
    void (*on_changing)(void *on_change_user_ptr, float nx, float ny);   // <- Function pointer that's called before x or y is changed (parameters are new x and new y but one will really only change at a time)
    void (*on_changed)(void *on_change_user_ptr);                        // <- Function pointer that's called after x or y is changed
}vector2_class_obj_t;

extern const mp_obj_type_t vector2_class_type;

mp_obj_t vector2_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // VECTOR2_H
