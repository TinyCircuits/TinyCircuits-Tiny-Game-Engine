#ifndef ENGINE_WAVE_SOUND_RESOURCE_H
#define ENGINE_WAVE_SOUND_RESOURCE_H

#include "py/obj.h"
#include "utility/engine_file.h"

// https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
typedef struct{
    mp_obj_base_t base;
    uint16_t channel_count;     // Number of channels in the wave file
    uint32_t sample_rate;       // Blocks per second
    uint16_t bits_per_sample;   // How many bits per sample
    uint8_t **channel_data;    // Array of pointers, each channel will have its data stored in ram
}wave_sound_resource_class_obj_t;

extern const mp_obj_type_t wave_sound_resource_class_type;

mp_obj_t wave_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_WAVE_SOUND_RESOURCE_H