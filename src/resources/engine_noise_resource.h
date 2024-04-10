#ifndef ENGINE_NOISE_RESOURCE_H
#define ENGINE_NOISE_RESOURCE_H

#include "py/obj.h"

#include "../lib/FastNoiseLite/C/FastNoiseLite.h"

typedef struct{
    mp_obj_base_t base;
    fnl_state fnl;
}noise_resource_class_obj_t;

extern const mp_obj_type_t noise_resource_class_type;

#endif  // ENGINE_NOISE_RESOURCE_H