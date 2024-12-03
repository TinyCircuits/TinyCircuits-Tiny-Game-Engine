#if defined(__EMSCRIPTEN__)
    #include <string.h>
    #include "engine_audio_web.h"


    void engine_audio_web_init(){
        
    }


    void engine_audio_web_channel_init(){
        
    }


    void engine_audio_web_setup_playback(){

    }


    void engine_audio_web_copy(uint8_t *dest, uint8_t *src, uint32_t count){
        memcpy(dest, src, count);
    }
#endif