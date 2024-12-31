#ifndef ENGINE_AUDIO_MODULE
#define ENGINE_AUDIO_MODULE

#include "engine_audio_channel.h"

#define ENGINE_AUDIO_SAMPLE_RATE 22050.0f
#define ENGINE_AUDIO_SAMPLE_DT 1.0f / ENGINE_AUDIO_SAMPLE_RATE

void engine_audio_apply_master_volume(float volume);
float engine_audio_get_master_volume();

void engine_audio_setup();
void engine_audio_setup_playback();

void engine_audio_play_on_channel(mp_obj_t sound_resource_obj, audio_channel_class_obj_t *channel, mp_obj_t loop_obj);
void engine_audio_reset();

#endif  // ENGINE_AUDIO_MODULE