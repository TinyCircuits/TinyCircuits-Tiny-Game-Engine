#ifndef ENGINE_SOUND_RESOURCE_BASE_H
#define ENGINE_SOUND_RESOURCE_BASE_H

#include "py/obj.h"
#include "py/mpthread.h"
#include "utility/engine_file.h"
#include "audio/engine_audio_channel.h"

// Forward declare since `resources/engine_audio_channel.h` and this file include each other
struct audio_channel_class_obj_t;

// All sound resources must conform to this structure.
// The audio playback engine expects these fields to
// exist and be populated correctly for the source
typedef struct sound_resource_base_class_obj_t{
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
    uint8_t *(*get_data)(void*, uint32_t, uint16_t*);               // Function used by playback engine to fill audio buffer
    void *extra_data;                                               // Each individual sound resource can attach extra data for its own use (not by the playback engine directly)
}sound_resource_base_class_obj_t;


#endif  // ENGINE_SOUND_RESOURCE_BASE_H