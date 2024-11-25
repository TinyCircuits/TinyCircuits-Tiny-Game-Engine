#ifndef ENGINE_AUDIO_UNIX_H
#define ENGINE_AUDIO_UNIX_H

#if defined(__unix__)
    void engine_audio_unix_init();
    void engine_audio_unix_setup_playback();
#endif

#endif  // ENGINE_AUDIO_UNIX_H