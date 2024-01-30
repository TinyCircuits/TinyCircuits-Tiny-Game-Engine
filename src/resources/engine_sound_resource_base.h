#ifndef ENGINE_SOUND_RESOURCE_BASE_H
#define ENGINE_SOUND_RESOURCE_BASE_H

#include "py/obj.h"
#include "py/mpthread.h"
#include "utility/engine_file.h"

// All sound resources must conform to this structure.
// The audio playback engine expects these fields to
// exist and be populated correctly for the source
typedef struct {
    mp_obj_base_t base;
    uint32_t total_sample_count;                                    // Value used by playback engine to know if it reached the end of the sound       
    uint32_t total_data_size;
    uint32_t sample_rate;                                           // Value used by playback engine to know how often to fetch new samples
    uint16_t bytes_per_sample;                                      // Value used by playback engine to know how many bytes are in a sample
    uint32_t (*get_data_buffer)(void*, uint8_t*, uint32_t, uint32_t);   // Function used by playback engine to fill audio buffer
    void *extra_data;                                               // Each individual sound resource can attach extra data for its own use (not by the playback engine directly)
    mp_thread_mutex_t mutex;                                        // Since audio playback is done on another core (core1), and some resources change their attributes based on user code, need to wait for each core to do their thing with each resource, per access
}sound_resource_base_class_obj_t;


#endif  // ENGINE_SOUND_RESOURCE_BASE_H