#ifndef ENGINE_MP_H
#define ENGINE_MP_H

#include "py/obj.h"

// Mimics `mp_load_attr` but for cases where the attribute may not exist. Returns `MP_OBJ_NULL` if not found (nothing like this is exposed to user code)
mp_obj_t engine_mp_load_attr_maybe(mp_obj_t base, qstr attr);

#endif