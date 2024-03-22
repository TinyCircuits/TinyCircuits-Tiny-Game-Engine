#ifndef ENGINE_AUDIO_CHANNEL_H
#define ENGINE_AUDIO_CHANNEL_H

#include "py/obj.h"
#include "py/mpthread.h"
#include <stdint.h>
#include "resources/engine_sound_resource_base.h"

#if defined(__unix__)

#elif defined(__arm__)
    #include "hardware/dma.h"
#endif

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

// Forward declare since `resources/engine_sound_resource_base.h` and this file include each other
typedef struct sound_resource_base_class_obj_t sound_resource_base_class_obj_t;

typedef struct audio_channel_class_obj_t{
    mp_obj_base_t base;
    sound_resource_base_class_obj_t *source;    // Source of the audio for the channel, currently
    uint32_t source_byte_offset;                // The total byte position inside the source that the ISR is using to start filling from
    float gain;                                 // Multiplier on each sample, 1.0 changes nothing
    float time;                                 // Where in the 'channel_source' the audio is being played from
    bool loop;                                  // Loop back to the start of the 'channel_source' at the end or set it to mp_const_none
    bool done;                                  // After starting a sound on this channel, this is set to true and then false when the end is reached (never set false in the case of 'looping' being true)
    uint8_t *buffers[2];                        // Dual buffers for audio, one gets DMA'ed to while to other is read from
    uint16_t buffers_ends[2];                   // Current of each buffer
    uint16_t buffers_byte_offsets[2];           // Current offset inside each buffer
    uint8_t reading_buffer_index;               // Index in 'buffers' of where audio samples should be picked from
    uint8_t filling_buffer_index;               // Index in 'buffers' of where audio samples should be taken from FLASH and stored in RAM
    bool busy;                                  // Used to indicate to the ISR that this channel shouldn't be used currently in the ISR

    #if defined(__unix__)

    #elif defined(__arm__)
        int dma_channel;                        // The DMA channel for this audio channel used for copying data from flash to RAM buffer in background
        dma_channel_config dma_config;          // Configuration for the DMA
    #endif
}audio_channel_class_obj_t;

extern const mp_obj_type_t audio_channel_class_type;

mp_obj_t audio_channel_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
mp_obj_t audio_channel_stop(mp_obj_t self_in);

#endif  // ENGINE_AUDIO_CHANNEL_H