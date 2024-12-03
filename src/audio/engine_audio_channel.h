#ifndef ENGINE_AUDIO_CHANNEL_H
#define ENGINE_AUDIO_CHANNEL_H

#include "py/obj.h"
#include "py/mpthread.h"
#include <stdint.h>


#if defined(__EMSCRIPTEN__)

#elif defined(__unix__)

#elif defined(__arm__)
    #include "hardware/dma.h"
#endif


// The number of total audio channels that
// can be active at a single time
#define CHANNEL_COUNT 4


// The total number of bytes dedicated to storing audio
// can be calculated as:
//      CHANNEL_COUNT * CHANNEL_BUFFER_LEN * 1 (1 since each channel has dual buffers)
//      4 * 1024 * 1 = 4096 bytes
// This buffer needs to large enough such that the target sample rate doesn't
// rip through it before the other buffer can be filled from flash. For example, if the
// target sample rate is 44100Hz and samples take 2 bytes then it takes:
//
// (1/44100) * (CHANNEL_BUFFER_LEN / BYTES_PER_SAMPLE)
// (1/44100) * (1024/2) = 11ms
//
// to get through the buffer. Hopefully we can take CHANNEL_BUFFER_LEN bytes out of flash
// and into RAM faster than that
#define CHANNEL_BUFFER_LEN 512

typedef struct audio_channel_class_obj_t{
    mp_obj_base_t base;
    mp_obj_t source;                            // Source of the audio for the channel, currently
    uint32_t source_byte_cursor;                // Where in the source the last copy ended (byte index)

    float gain;                                 // Multiplier on each sample, 1.0 changes nothing
    float time;                                 // Where in the 'channel_source' the audio is being played from
    float amplitude;                            // Last amplitude on this played on this channel (before clamping or gain)
    bool loop;                                  // Loop back to the start of the 'channel_source' at the end or set it to mp_const_none
    bool done;                                  // After starting a sound on this channel, this is set to true and then false when the end is reached (never set false in the case of 'looping' being true)

    uint8_t *buffers[2];                        // Dual buffers for audio, one gets DMA'ed to while the other is read from (uint8_t instead of float since most data not float)
    uint16_t buffers_data_len[2];               // Current end index for each buffer
    uint16_t buffers_byte_cursor[2];            // Current offset inside each buffer to read from (byte index)
    uint8_t buffer_to_fill_index;               // Index (0 or 1) into `buffers` indicating which buffer to fill with data from its source
    uint8_t buffer_to_read_index;               // Index (0 or 1) into `buffers` indicating which buffer to read data from inside the channel
    bool busy;                                  // Used to indicate to the ISR that this channel shouldn't be used currently in the ISR

    #if defined(__EMSCRIPTEN__)
        // Nothing unique needed
    #elif defined(__unix__)
        // Nothing unique needed
    #elif defined(__arm__)
        int dma_copy_channel;                    // The DMA channel for this audio channel used for copying data from flash to RAM buffer in background
        dma_channel_config dma_copy_config;      // Configuration for the DMA
    #endif
}audio_channel_class_obj_t;

extern const mp_obj_type_t audio_channel_class_type;

mp_obj_t audio_channel_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
bool audio_channel_get_samples(audio_channel_class_obj_t *channel, float *output, uint32_t count);
void audio_channeL_buffer_reset(audio_channel_class_obj_t *channel);
bool audio_channel_fill_internal_buffer(audio_channel_class_obj_t *channel, uint8_t buffer_to_fill_index);
mp_obj_t audio_channel_stop(mp_obj_t self_in);

#endif  // ENGINE_AUDIO_CHANNEL_H