#ifndef ENGINE_ANIMATION_DELAY_H
#define ENGINE_ANIMATION_DELAY_H

#include "py/obj.h"
#include "utility/linked_list.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_t object;
    float delay;
    float time;
    bool finished;
    mp_obj_t tick;
    mp_obj_t after;
    void *self;
    linked_list_node *list_node;
}delay_class_obj_t;

extern const mp_obj_type_t delay_class_type;

#endif  // ENGINE_ANIMATION_DELAY_H