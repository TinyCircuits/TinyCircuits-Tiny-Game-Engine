#ifndef ENGINE_AUDIO_UNIX_H
#define ENGINE_AUDIO_UNIX_H

#if defined(__unix__)
    #include <stdint.h>

    void engine_audio_unix_init();
    void engine_audio_unix_channel_init();
    void engine_audio_unix_setup_playback();
    void engine_audio_unix_copy(uint8_t *dest, uint8_t *src, uint32_t count);
#endif

#endif  // ENGINE_AUDIO_UNIX_H