#if defined(__EMSCRIPTEN__)
    #include <stdint.h>
    #include <string.h>
    #include <signal.h>
    #include <sys/time.h>
    #include <emscripten.h>
    #include <emscripten/html5.h>

    #include "engine_audio_web.h"
    #include "debug/debug_print.h"
    #include "audio/engine_audio_module.h"
    #include "audio/engine_audio_channel.h"


    struct itimerval timer;


    // Callback that is invoked at engine audio sample rate
    bool alarm_handler(double time, void *userData){
        // ENGINE_PRINTF("AUDIO TEST 1\n");
        return true;
    }


    void engine_audio_web_init_one_time(){
        // ENGINE_PRINTF("AUDIO TEST 0\n");

        // emscripten_set_timeout_loop(alarm_handler, ((1.0 / (double)ENGINE_AUDIO_SAMPLE_RATE) * 1000.0), NULL);

        // signal(SIGALRM, sigint_handler);
        
        // timer.it_value.tv_sec = 0; 
        // timer.it_value.tv_usec = (long int)((1.0f / (float)ENGINE_AUDIO_SAMPLE_RATE) * 1000000.0f);
        // timer.it_interval.tv_sec = 0; 
        // timer.it_interval.tv_usec = (long int)((1.0f / (float)ENGINE_AUDIO_SAMPLE_RATE) * 1000000.0f);;
        // setitimer(ITIMER_REAL, &timer, NULL);
    }


    void engine_audio_web_channel_init_one_time(){
        
    }


    void engine_audio_web_setup_playback(){

    }


    void engine_audio_web_copy(uint8_t *dest, uint8_t *src, uint32_t count){
        memcpy(dest, src, count);
    }
#endif