#ifndef ENGINE_WAVE_SOUND_RESOURCE_H
#define ENGINE_WAVE_SOUND_RESOURCE_H

#include "py/obj.h"


typedef struct wave_sound_resource_class_obj_t{
    mp_obj_base_t base;
    bool in_ram;
    uint32_t total_sample_count;                                    // Value used by playback engine to know if it reached the end of the sound       
    uint32_t total_data_size;
    uint32_t sample_rate;                                           // Value used by playback engine to know how often to fetch new samples
    uint8_t play_counter_max;                                       // How many times the 22050Hz interrupt needs to be called before getting the next sample
    uint8_t play_counter;                                           // Based on the playback sample rate of the engine and the source, this tracks when the next sample from the source should be played
    float last_sample;                                              // Keep the last sample to return it for times when it is not time to return a new sample
    uint16_t bytes_per_sample;                                      // Value used by playback engine to know how many bytes are in a sample
    struct audio_channel_class_obj_t *channel;                      // If being played by a channel, then this is the channel that is playing it (IMPORTANT: need this link so that when this source is deleted it can remove itself from the channel as a source by setting itself NULL)
    void *data;                                                     // Wave 8 or 16 bit PCM sound data
}wave_sound_resource_class_obj_t;


// https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
extern const mp_obj_type_t wave_sound_resource_class_type;

mp_obj_t wave_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
uint32_t wave_fill_dest(wave_sound_resource_class_obj_t *wave, audio_channel_class_obj_t *channel, uint8_t *output, uint32_t byte_count, bool *complete);

#endif  // ENGINE_WAVE_SOUND_RESOURCE_H