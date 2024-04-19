#ifndef ENGINE_AUDIO_MODULE
#define ENGINE_AUDIO_MODULE

#define ENGINE_AUDIO_SAMPLE_RATE 22050.0f
#define ENGINE_AUDIO_SAMPLE_DT 1.0f / ENGINE_AUDIO_SAMPLE_RATE

void engine_audio_setup();
void engine_audio_setup_playback();
void engine_audio_stop();

#endif  // ENGINE_AUDIO_MODULE