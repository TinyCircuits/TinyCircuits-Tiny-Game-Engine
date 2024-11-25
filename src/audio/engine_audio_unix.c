#if defined(__unix__)
    #include "engine_audio_unix.h"
    #include <SDL2/SDL.h>

    // Main audio playback object
    SDL_AudioSpec audio;


    void engine_audio_unix_init(){
        audio.freq = 22050;
        audio.format = AUDIO_U16;
    }


    void engine_audio_unix_setup_playback(){

    }
#endif