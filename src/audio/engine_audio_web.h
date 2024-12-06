#ifndef ENGINE_AUDIO_WEB_H
#define ENGINE_AUDIO_WEB_H

#if defined(__EMSCRIPTEN__)
    void engine_audio_web_init_one_time();
    void engine_audio_web_channel_init_one_time();
    void engine_audio_web_setup_playback();
    void engine_audio_web_copy(uint8_t *dest, uint8_t *src, uint32_t count);
#endif

#endif  // ENGINE_AUDIO_WEB_H