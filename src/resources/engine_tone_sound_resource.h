#ifndef ENGINE_TONE_SOUND_RESOURCE_H
#define ENGINE_TONE_SOUND_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_defines.h"

typedef struct{
    mp_obj_base_t base;
    audio_channel_class_obj_t *channel;

    float frequency;
    float omega;
    float time;
    bool busy;
}tone_sound_resource_class_obj_t;

extern const mp_obj_type_t tone_sound_resource_class_type;

mp_obj_t tone_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
void tone_sound_resource_set_frequency(tone_sound_resource_class_obj_t *self, float frequency);
uint32_t tone_fill_dest(tone_sound_resource_class_obj_t *tone, audio_channel_class_obj_t *channel, uint8_t *output, uint32_t byte_count, bool *complete);

#endif  // ENGINE_TONE_SOUND_RESOURCE_H