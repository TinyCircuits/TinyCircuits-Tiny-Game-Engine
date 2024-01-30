#ifndef ENGINE_AUDIO_CHANNEL_H
#define ENGINE_AUDIO_CHANNEL_H

#include "py/obj.h"
#include <stdint.h>
#include "resources/engine_sound_resource_base.h"


// The number of total audio channels that
// can be active at a single time
#define CHANNEL_COUNT 4


// The total number of bytes dedicated to storing audio
// can be calculated as:
//      CHANNEL_COUNT * CHANNEL_BUFFER_SIZE * 1 (1 since each channel has dual buffers)
//      4 * 1024 * 1 = 4096 bytes
// This buffer needs to large enough such that the target sample rate doesn't
// rip through it before the other buffer can be filled from flash. For example, if the
// target sample rate is 44100Hz and samples take 2 bytes then it takes:
//
// (1/44100) * (CHANNEL_BUFFER_SIZE / BYTES_PER_SAMPLE)
// (1/44100) * (1024/2) = 11ms
//
// to get through the buffer. Hopefully we can take CHANNEL_BUFFER_SIZE bytes out of flash
// and into RAM faster than that
#define CHANNEL_BUFFER_SIZE 1024


typedef struct{
    mp_obj_base_t base;
    sound_resource_base_class_obj_t *source;    // Source of the audio for the channel, currently
    uint32_t source_byte_offset;                // The total byte position inside the source that the ISR is using to start filling from
    float gain;                                 // Multiplier on each sample, 1.0 changes nothing
    float time;                                 // Where in the 'channel_source' the audio is being played from
    bool looping;                               // Loop back to the start of the 'channel_source' at the end or set it to mp_const_none
    bool done;                                  // After starting a sound on this channel, this is set to true and then false when the end is reached (never set false in the case of 'looping' being true)
    uint8_t *buffer;                            // Buffer to hold data from other locations (flash or generated)
    uint16_t buffer_byte_offset;                // Where we are in the buffer pointed to by 'fill_buffer_index'. Loop back to 0 when reach sample count stored in 'source'
}audio_channel_class_obj_t;

extern const mp_obj_type_t audio_channel_class_type;

mp_obj_t audio_channel_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_AUDIO_CHANNEL_H