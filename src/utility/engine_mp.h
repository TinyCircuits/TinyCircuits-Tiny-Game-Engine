#ifndef ENGINE_MP_H
#define ENGINE_MP_H

#include "py/obj.h"
#include "py/mpconfig.h"

// From py/objfloat.c since not exposed
typedef struct _mp_obj_float_t {
    mp_obj_base_t base;
    mp_float_t value;
} mp_obj_float_t;

// Mimics `mp_load_attr` but for cases where the attribute may not exist. Returns `MP_OBJ_NULL` if not found (nothing like this is exposed to user code)
mp_obj_t engine_mp_load_attr_maybe(mp_obj_t base, qstr attr);

#endif