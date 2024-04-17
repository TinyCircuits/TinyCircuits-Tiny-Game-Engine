#ifndef ENGINE_TONE_SOUND_RESOURCE_H
#define ENGINE_TONE_SOUND_RESOURCE_H

#include "py/obj.h"
#include "engine_sound_resource_base.h"
#include "utility/engine_defines.h"

typedef struct{
    mp_obj_base_t base;
    audio_channel_class_obj_t *channel;

    float frequency;
    float omega;
    float dt;
    float time;
}tone_sound_resource_class_obj_t;

extern const mp_obj_type_t tone_sound_resource_class_type;

float ENGINE_FAST_FUNCTION(tone_sound_resource_get_sample)(tone_sound_resource_class_obj_t *self);
mp_obj_t tone_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_TONE_SOUND_RESOURCE_H