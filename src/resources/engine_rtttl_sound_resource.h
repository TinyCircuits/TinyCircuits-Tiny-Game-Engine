#ifndef ENGINE_RTTTL_SOUND_RESOURCE_H
#define ENGINE_RTTTL_SOUND_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_defines.h"
#include "audio/engine_audio_channel.h"

typedef struct{
    mp_obj_base_t base;
    audio_channel_class_obj_t *channel;
    mp_obj_t tone;
    mp_obj_t data;

    uint16_t note_count;
    uint32_t note_cursor;

    uint32_t interrupt_samples_until_next;
    uint32_t interrupt_samples_counting;
}rtttl_sound_resource_class_obj_t;

extern const mp_obj_type_t rtttl_sound_resource_class_type;

float ENGINE_FAST_FUNCTION(rtttl_sound_resource_get_sample)(rtttl_sound_resource_class_obj_t *self, bool *complete);
mp_obj_t rtttl_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
uint32_t rtttl_fill_dest(rtttl_sound_resource_class_obj_t *rtttl, audio_channel_class_obj_t *channel, uint8_t *output, uint32_t byte_count, bool *complete);
uint32_t rtttl_convert(rtttl_sound_resource_class_obj_t *rtttl, uint8_t *channel_buffer, float *output, uint32_t sample_count, float volume);

#endif  // ENGINE_RTTTL_SOUND_RESOURCE_H