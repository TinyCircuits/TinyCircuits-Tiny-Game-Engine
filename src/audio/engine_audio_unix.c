#if defined(__unix__)
    #include "engine_audio_unix.h"
    #include "audio/engine_audio_module.h"
    #include "audio/engine_audio_channel.h"
    #include "debug/debug_print.h"
    #include <string.h>
    #include <SDL2/SDL.h>

    // Main audio playback object
    SDL_AudioSpec audio;


    void engine_audio_fill(void *user_data, Uint8 *to_fill_buffer, int buffer_len){
        bool complete = false;
        audio_channel_class_obj_t *channel = engine_audio_get_channel(0);
        float volume = channel->gain * engine_audio_get_game_volume() * engine_audio_get_master_volume();
        audio_channel_get_samples(channel, (float*)to_fill_buffer, buffer_len, volume, &complete);
    }


    void engine_audio_unix_init(){
        // audio.freq = 11025;
        audio.freq = 22050;
        audio.format = AUDIO_F32;
        audio.channels = 1;
        audio.samples = CHANNEL_BUFFER_LEN;
        audio.callback = engine_audio_fill;
        audio.userdata = NULL;

        if(SDL_OpenAudio(&audio, NULL) < 0 ){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineAudio UNIX: ERROR: Could not open audio! %s"), SDL_GetError());
        }
    }


    void engine_audio_unix_channel_init(){
        
    }


    void engine_audio_unix_setup_playback(){
        SDL_PauseAudio(0);
    }


    void engine_audio_unix_copy(uint8_t *dest, uint8_t *src, uint32_t count){
        memcpy(dest, src, count);
    }
#endif