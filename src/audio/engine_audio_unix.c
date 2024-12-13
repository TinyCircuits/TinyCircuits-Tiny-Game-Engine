#if defined(__unix__) && !defined(__EMSCRIPTEN__)   // When building for web unix is defined
    #include "engine_audio_unix.h"
    #include "audio/engine_audio_module.h"
    #include "audio/engine_audio_channel.h"
    #include "debug/debug_print.h"
    #include <string.h>
    #include <signal.h>
    #include <sys/time.h>
    #include <pthread.h>
    #include <SDL2/SDL.h>

    // Main audio playback object
    SDL_AudioSpec sdl_audio_spec;
    SDL_AudioDeviceID sdl_audio_device;
    // struct itimerval timer;
    

    // void *copy_thread(void *channel_ptr){

    //     while(true){
    //         if(channel_ptr == NULL){
    //             continue;
    //         }

    //         audio_channel_class_obj_t *channel = (audio_channel_class_obj_t*)channel_ptr;

    //         if(channel->copy_dest == NULL || channel->copy_src == NULL || channel->copy_count == 0){
    //             continue;
    //         }

    //         // Lock access to `dest` and `src` while working with them
    //         pthread_mutex_lock(&channel->mutex);

    //         memcpy(channel->copy_dest, channel->copy_src, channel->copy_count);
    //         channel->copy_dest = NULL;
    //         channel->copy_src = NULL;
    //         channel->copy_count = 0;

    //         pthread_mutex_unlock(&channel->mutex);
    //     }

    //     return NULL;
    // }


    // // Callback that is invoked at engine audio sample rate
    // void sigint_handler(int signum){

    //     bool play = false;    // Set `true` if at least one channel is ready to play
    //     float output = engine_audio_get_mixed_output_sample(&play);

    //     if(play){
    //         SDL_QueueAudio(sdl_audio_device, &output, 4);
    //     }

    //     setitimer(ITIMER_REAL, &timer, NULL);
    // }





    void fill_audio(void *udata, Uint8 *stream, int len){
        float *output = (float*)stream;
        uint32_t sample_count = len / 4;
        bool play = false;

        for(uint32_t isx=0; isx<sample_count; isx++){
            output[isx] = engine_audio_get_mixed_output_sample(&play);
        }
    }


    void engine_audio_unix_init_one_time(){
        // signal(SIGALRM, sigint_handler);
        
        // timer.it_value.tv_sec = 0; 
        // timer.it_value.tv_usec = (__suseconds_t)((1.0f / (float)ENGINE_AUDIO_SAMPLE_RATE) * 1000000.0f); 
        // timer.it_interval.tv_sec = 0; 
        // timer.it_interval.tv_usec = (__suseconds_t)((1.0f / (float)ENGINE_AUDIO_SAMPLE_RATE) * 1000000.0f);
        // setitimer(ITIMER_REAL, &timer, NULL);

        sdl_audio_spec.freq = ENGINE_AUDIO_SAMPLE_RATE;
        sdl_audio_spec.format = AUDIO_F32;
        sdl_audio_spec.channels = 1;
        sdl_audio_spec.samples = CHANNEL_BUFFER_LEN;
        sdl_audio_spec.callback = fill_audio;
        sdl_audio_spec.userdata = NULL;

        SDL_AudioInit(NULL);
        sdl_audio_device = SDL_OpenAudioDevice(NULL, false, &sdl_audio_spec, NULL, false);

        if(sdl_audio_device == 0){
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineAudio UNIX: ERROR: Could not open audio device! '%s'"), SDL_GetError());
        }
    }


    void engine_audio_unix_channel_init_one_time(audio_channel_class_obj_t *channel){
        // // Set these so that the copy thread
        // // doesn't attempt a copy on non-set pointers
        // channel->copy_dest = NULL;
        // channel->copy_src = NULL;
        // channel->copy_count = 0;

        // pthread_mutex_init(&channel->mutex, NULL);

        // int result = pthread_create(&channel->copy_thread_id, NULL, copy_thread, channel);

        // if(result != 0){
        //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineAudio: UNIX: ERROR: Could not create thread runner for copying data!"));
        // }else{
        //     ENGINE_INFO_PRINTF("Created copy thread RUNNER");
        // }
    }


    void engine_audio_unix_setup_playback(){
        SDL_PauseAudioDevice(sdl_audio_device, 0);
    }


    void engine_audio_unix_copy(audio_channel_class_obj_t *channel, uint8_t *dest, uint8_t *src, uint32_t count){
        memcpy(dest, src, count);
        // // Wait for mutex to unlock in case thread
        // // already locked access the same `dest and src`
        // pthread_mutex_lock(&channel->mutex);

        // channel->copy_count = count;
        // channel->copy_dest = dest;
        // channel->copy_src = src;

        // pthread_mutex_unlock(&channel->mutex);
    }
#endif