#ifndef ENGINE_TONE_SOUND_RESOURCE_H
#define ENGINE_TONE_SOUND_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_defines.h"
#include "audio/engine_audio_channel.h"


// ENGINE_AUDIO_SAMPLE_RATE          [samples/second] or [Hz]
// ENGINE_AUDIO_SAMPLE_RATE_PERIOD   [second/samples]
// CHANNEL_BUFFER_LEN                [samples]
// ENGINE_TONE_MINIMUM_HZ            [HZ]
//
// Therefore: ENGINE_TONE_MINIMUM_HZ  =  1.0 / ENGINE_AUDIO_SAMPLE_RATE_PERIOD * CHANNEL_BUFFER_LEN  =  1.0 / [second/samples] * [samples]  =  1.0 / [second]  = [Hz]
#define ENGINE_TONE_MINIMUM_HZ     1.0f / (ENGINE_AUDIO_SAMPLE_RATE_PERIOD * CHANNEL_BUFFER_LEN)
#define ENGINE_TONE_MINIMUM_PERIOD 1.0f / ENGINE_TONE_MINIMUM_HZ

// ENGINE_TONE_MINIMUM_PERIOD        [second/1.0]
// ENGINE_AUDIO_SAMPLE_RATE_PERIOD   [second/samples]
//
// Therefore: ENGINE_TONE_BUFFER_LEN  =  ENGINE_TONE_MINIMUM_PERIOD / ENGINE_AUDIO_SAMPLE_RATE_PERIOD  =  [second/1.0] / [second/samples]  =  [second/1.0] * [samples/second]  = [samples]
#define ENGINE_TONE_BUFFER_LEN ENGINE_TONE_MINIMUM_PERIOD / ENGINE_AUDIO_SAMPLE_RATE_PERIOD


typedef struct{
    mp_obj_base_t base;
    audio_channel_class_obj_t *channel;

    float frequency;
    float omega;
    float time;
    bool busy;
}tone_sound_resource_class_obj_t;

extern const mp_obj_type_t tone_sound_resource_class_type;

void tone_sound_resource_set_frequency(tone_sound_resource_class_obj_t *self, float frequency);
mp_obj_t tone_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
uint32_t tone_fill_dest(tone_sound_resource_class_obj_t *tone, audio_channel_class_obj_t *channel, uint8_t *output, uint32_t byte_count, bool *complete);
uint32_t tone_convert(tone_sound_resource_class_obj_t *tone, uint8_t *channel_buffer, float *output, uint32_t sample_count, float volume);

#endif  // ENGINE_TONE_SOUND_RESOURCE_H