#ifndef ENGINE_ANIMATION_TWEEN_H
#define ENGINE_ANIMATION_TWEEN_H

#include "py/obj.h"
#include "utility/linked_list.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_t value;
    float duration;
    uint8_t loop_type;
    uint8_t tween_type;
    float time;
    mp_obj_t tick;

    float initial_0;
    float end_0;


    void *self;
    linked_list_node *list_node;

    bool finished;
}tween_class_obj_t;

extern const mp_obj_type_t tween_class_type;

#endif  // ENGINE_ANIMATION_TWEEN_H