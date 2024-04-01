#ifndef ENGINE_ANIMATION_TWEEN_H
#define ENGINE_ANIMATION_TWEEN_H

#include "py/obj.h"
#include "utility/linked_list.h"

typedef struct{
    mp_obj_base_t base;
    mp_obj_t object;
    qstr attr;
    float duration;
    uint8_t loop_type;
    uint8_t ease_type;
    uint8_t tween_type;

    float time;
    float speed;

    mp_obj_t tick;

    mp_obj_t after;
    bool after_called;

    float initial_0;
    float initial_1;
    float initial_2;

    float end_0;
    float end_1;
    float end_2;

    void *self;
    linked_list_node *list_node;

    uint8_t tween_direction;  // Either 0.0 or 1.0f depending on direction and if in ping pong mode
    float ping_pong_multiplier; // Either 1.0 or 2.0 depending on if in ping pong mode

    bool finished;
    bool paused;
}tween_class_obj_t;

extern const mp_obj_type_t tween_class_type;

#endif  // ENGINE_ANIMATION_TWEEN_H