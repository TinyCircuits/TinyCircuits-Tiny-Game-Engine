#ifndef ENGINE_AUDIO_MODULE
#define ENGINE_AUDIO_MODULE

#include "engine_audio_channel.h"

#define ENGINE_AUDIO_SAMPLE_RATE 22050.0f
#define ENGINE_AUDIO_SAMPLE_RATE_PERIOD 1.0f / ENGINE_AUDIO_SAMPLE_RATE

audio_channel_class_obj_t *engine_audio_get_channel(uint8_t channel_index);

void engine_audio_init_one_time();

void engine_audio_apply_master_volume(float volume);
float engine_audio_get_master_volume();

void engine_audio_apply_game_volume(float volume);
float engine_audio_get_game_volume();

void engine_audio_setup_playback();

float engine_audio_get_mixed_output_sample();

void engine_audio_play_on_channel(mp_obj_t sound_resource_obj, audio_channel_class_obj_t *channel, mp_obj_t loop_obj);
void engine_audio_reset();

#endif  // ENGINE_AUDIO_MODULE