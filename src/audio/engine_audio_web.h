#ifndef ENGINE_AUDIO_WEB_H
#define ENGINE_AUDIO_WEB_H

#if defined(__EMSCRIPTEN__)
    void engine_audio_web_init();
    void engine_audio_web_setup_playback();
#endif

#endif  // ENGINE_AUDIO_WEB_H