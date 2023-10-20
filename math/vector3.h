#ifndef VECTOR3_H
#define VECTOR3_H

#include "py/obj.h"

typedef struct{
    mp_obj_base_t base;
    float x;
    float y;
    float z;
}vector3_class_obj_t;

const mp_obj_type_t vector3_class_type;

#endif  // VECTOR3_H