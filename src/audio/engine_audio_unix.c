#if defined(__unix__)
    #include "engine_audio_unix.h"
    #include "audio/engine_audio_module.h"
    #include "audio/engine_audio_channel.h"
    #include "debug/debug_print.h"
    #include <string.h>
    #include <signal.h>
    #include <sys/time.h>
    #include <SDL2/SDL.h>

    // Main audio playback object
    SDL_AudioSpec sdl_audio_spec;
    SDL_AudioDeviceID sdl_audio_device;

    struct itimerval timer;
    

    void sigint_handler(int signum){
        bool play = false;    // Set `true` if at least one channel is ready to play
        float output = engine_audio_get_mixed_output_sample(&play);

        if(play){
            SDL_QueueAudio(sdl_audio_device, &output, 4);
        }

        setitimer(ITIMER_REAL, &timer, NULL);
    }


    void engine_audio_unix_init_one_time(){
        signal(SIGALRM, sigint_handler);
        
        timer.it_value.tv_sec = 0; 
        timer.it_value.tv_usec = (__suseconds_t)((1.0f / (float)ENGINE_AUDIO_SAMPLE_RATE) * 1000000.0f); 
        timer.it_interval.tv_sec = 0; 
        timer.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);

        // // audio.freq = 11025;
        sdl_audio_spec.freq = ENGINE_AUDIO_SAMPLE_RATE;
        sdl_audio_spec.format = AUDIO_F32;
        sdl_audio_spec.channels = 1;
        sdl_audio_spec.samples = CHANNEL_BUFFER_LEN;
        sdl_audio_spec.callback = NULL;
        sdl_audio_spec.userdata = NULL;

        SDL_AudioInit(NULL);
        sdl_audio_device = SDL_OpenAudioDevice(NULL, false, &sdl_audio_spec, NULL, false);

        if(sdl_audio_device == 0){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineAudio UNIX: ERROR: Could not open audio device! '%s'"), SDL_GetError());
        }
    }


    void engine_audio_unix_channel_init_one_time(audio_channel_class_obj_t *channel){
        
    }


    void engine_audio_unix_setup_playback(){
        SDL_PauseAudioDevice(sdl_audio_device, 0);
    }


    void engine_audio_unix_copy(uint8_t *dest, uint8_t *src, uint32_t count){
        memcpy(dest, src, count);
    }
#endif