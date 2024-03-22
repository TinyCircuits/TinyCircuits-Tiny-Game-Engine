#ifndef ENGINE_WAVE_SOUND_RESOURCE_H
#define ENGINE_WAVE_SOUND_RESOURCE_H

#include "py/obj.h"
#include "engine_sound_resource_base.h"

// https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
extern const mp_obj_type_t wave_sound_resource_class_type;

mp_obj_t wave_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);

#endif  // ENGINE_WAVE_SOUND_RESOURCE_H