#ifndef ENGINE_AUDIO_UNIX_H
#define ENGINE_AUDIO_UNIX_H

#if defined(__unix__)
    #include <stdint.h>
    #include "audio/engine_audio_channel.h"

    void engine_audio_unix_init_one_time();
    void engine_audio_unix_channel_init_one_time(audio_channel_class_obj_t *channel);
    void engine_audio_unix_setup_playback();
    void engine_audio_unix_copy(audio_channel_class_obj_t *channel, uint8_t *dest, uint8_t *src, uint32_t count);
#endif

#endif  // ENGINE_AUDIO_UNIX_H