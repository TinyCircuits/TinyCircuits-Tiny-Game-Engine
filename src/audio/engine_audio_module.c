#include "py/obj.h"
#include "py/mpthread.h"
#include "engine_audio_module.h"
#include "engine_audio_channel.h"
#include "resources/engine_sound_resource_base.h"
#include "debug/debug_print.h"
#include <stdlib.h>
#include <string.h>
#include "math/engine_math.h"



// 8 audio channels. Since audio is large it has to stay in
// flash, but flash is slow to get data from which is a problem
// when sample retrieval time/latency needs to be small to keep
// on track at the relatively high sample rate good audio requires.
// DMA is used to copy data into one of the dual buffer pairs
// each audio channel gets but there's only 12 channels on RP2040
// and one is used for the screen.
//
// Audio channels need to be stored in root pointers so
// they are not collected by gc
// https://github.com/orgs/micropython/discussions/9312#discussioncomment-3644172
// Use MP_STATE_PORT to get the objects from root pointers
MP_REGISTER_ROOT_POINTER(mp_obj_t channels[CHANNEL_COUNT]);

// The master volume that all mixed samples are scaled by
float master_volume = 1.0f;


#if defined(__unix__)
    // Nothing to do
#elif defined(__arm__)
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/structs/xip_ctrl.h"
    #include "hardware/pwm.h"
    #include "hardware/timer.h"
    #include "pico/multicore.h"

    // alarm_pool_t *core1_alarm_pool = NULL;

    // pg. 542: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
    // https://github.com/raspberrypi/pico-examples/blob/master/timer/hello_timer/hello_timer.c#L11-L57
    // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#rpipdb65a0bdce0635d95877
    // https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#interrupt-numbers
    struct repeating_timer repeating_audio_timer;

    void __no_inline_not_in_flash_func(engine_audio_handle_buffer)(audio_channel_class_obj_t *channel){
        // When 'buffer_byte_offset = 0' that means the buffer hasn't been filled before, fill it (see that after this function it is immediately incremented)
        // When 'buffer_byte_offset >= channel->buffer_end' that means the index has run out of data, fill it with more
        if(channel->buffer_byte_offset == 0 || channel->buffer_byte_offset >= channel->buffer_end){
            // Reset for the second case above
            channel->buffer_byte_offset = 0;

            // Using the sound resource base, fill this channel's
            // buffer with audio data from the source resource
            // channel->buffer_end = channel->source->fill_buffer(channel->source, channel->buffer, channel->source_byte_offset, CHANNEL_BUFFER_SIZE);

            uint8_t *current_source_data = channel->source->get_data(channel, CHANNEL_BUFFER_SIZE, &channel->buffer_end);

            // memcpy((uint8_t*)channel->buffer, (uint8_t*)current_source_data, channel->buffer_end);

            // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/flash/xip_stream/flash_xip_stream.c#L45-L48
            // while (!(xip_ctrl_hw->stat & XIP_STAT_FIFO_EMPTY))
            //     (void) xip_ctrl_hw->stream_fifo;
            // xip_ctrl_hw->stream_addr = (uint32_t)current_source_data;
            // xip_ctrl_hw->stream_ctr = channel->buffer_end;

            // https://github.com/raspberrypi/pico-examples/blob/master/flash/xip_stream/flash_xip_stream.c#L58-L70
            dma_channel_configure(
                channel->dma_channel,     // Channel to be configured
                &channel->dma_config,     // The configuration we just created
                channel->buffer,          // The initial write address
                current_source_data,      // The initial read address
                channel->buffer_end,      // Number of transfers; in this case each is 1 byte
                true                      // Start immediately
            );

            dma_channel_wait_for_finish_blocking(channel->dma_channel);

            // Filled amount will always be equal to or less than to 
            // 0 the 'size' passed to 'fill_buffer'. In the case it was
            // filled with something, increment to the amount filled
            // further. In the case it is filled with nothing, that means
            // the last fill made us reach the end of the source data,
            // figure out if this channel should stop or loop. If loop,
            // run again right away to fill with more data after resetting
            // 'source_byte_offset' 
            if(channel->buffer_end > 0){
                channel->source_byte_offset += channel->buffer_end;
            }else{
                // Gets reset no matter what, whether looping or not
                channel->source_byte_offset = 0;

                // If not looping, disable/remove the source and stop this
                // channel from being played, otherwise, fill with start data
                if(channel->looping == false){
                    channel->source = NULL;
                }else{
                    // Run right away to fill buffer with starting data since looping
                    engine_audio_handle_buffer(channel);
                }
            }
        }
    }


    // Samples each channel, adds, normalizes, and sets PWM
    bool __no_inline_not_in_flash_func(repeating_audio_callback)(struct repeating_timer *t){
        float temp_sample = 0;
        float total_sample = 0;
        uint8_t active_channel_count = 0;

        for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){

            audio_channel_class_obj_t *channel = MP_STATE_PORT(channels[icx]);

            // For each channel, make sure we have access to it now
            // and not core0 (which may be reading/writing to any
            // of the used attributes)
            mp_thread_mutex_lock(&channel->mutex, true);

            sound_resource_base_class_obj_t *source = channel->source;

            // Go over every channel and check if set to something usable
            if(source != NULL){

                // For each source, make sure we have access to it now
                // and not core0 (which may be reading/writing to any
                // of the used attributes)
                mp_thread_mutex_lock(&source->mutex, true);

                // Another active channel
                active_channel_count += 1;

                // Fill buffer with data whether first time or looping
                engine_audio_handle_buffer(channel);

                // Add samples to total
                switch(source->bytes_per_sample){
                    case 1:
                    {
                        temp_sample = (float)channel->buffer[channel->buffer_byte_offset];                      // Get 8-bit sample
                        temp_sample = temp_sample / (float)UINT8_MAX;                                           // Scale from 0 ~ 255 to 0.0 ~ 1.0
                        temp_sample = temp_sample * channel->gain;                                              // Scale sample by channel gain
                        temp_sample = (engine_math_clamp(temp_sample, 0.0f, 1.0f) * 512.0f * master_volume);    // Clamp volume scaled sample and scale from 0.0 ~ 1.0 to 0.0 to 512.0 (512 scaled by master_volume)
                    }
                    break;
                    case 2:
                    {   
                        temp_sample = (int16_t)(channel->buffer[channel->buffer_byte_offset+1] << 8) + channel->buffer[channel->buffer_byte_offset];    // Get 16-bit sample
                        temp_sample = temp_sample / (float)UINT16_MAX;                                                                                  // Scale from 0 ~ 65535 to 0.0 ~ 1.0
                        temp_sample = temp_sample * channel->gain;                                                                                      // Scale sample by channel gain
                        temp_sample = (engine_math_clamp(temp_sample, 0.0f, 1.0f) * 512.0f * master_volume);                                            // Clamp volume scaled sample and scale from 0.0 ~ 1.0 to 0.0 to 512.0 (512 scaled by master_volume)
                    }
                    break;
                    default:
                        ENGINE_ERROR_PRINTF("Audio source with %d bytes per sample is not supported!", source->bytes_per_sample);
                }

                // Add the sample to the total
                total_sample += temp_sample;

                // Make sure to grab the next sample next time
                channel->buffer_byte_offset += source->bytes_per_sample;

                // Calculate the current time that we're at in the channel's source
                // t_current =  (1 / sample_rate [1/s]) * (source_byte_offset / bytes_per_sample)
                channel->time = (1.0f / source->sample_rate) * (channel->source_byte_offset / source->bytes_per_sample);
            }

            mp_thread_mutex_unlock(&channel->mutex);
            mp_thread_mutex_unlock(&source->mutex);
        }
        
        // https://dsp.stackexchange.com/questions/3581/algorithms-to-mix-audio-signals-without-clipping: Viktor was wrong!
        // https://www.google.com/search?q=mixing+n+number+of+channels+audio+stackexchange&rlz=1C1GCEA_enUS850US850&oq=mixing+n+number+of+channels+audio+stackexchange&gs_lcrp=EgZjaHJvbWUyBggAEEUYOTIJCAEQIRgKGKABMgcIAhAhGKsC0gEINjk5OWowajeoAgCwAgA&sourceid=chrome&ie=UTF-8
        // https://electronics.stackexchange.com/questions/193983/mixing-audio-in-a-microprocessor       
        if(active_channel_count > 0){
            pwm_set_gpio_level(23, (uint32_t)(total_sample / active_channel_count));
        }

        return true;
    }
#endif


void engine_audio_setup_playback(){
    #if defined(__unix__)
        // Nothing to do
    #elif defined(__arm__)
        // Need to make new alarm pool for timers on this core
        // core1_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(CHANNEL_COUNT);

        // Setup timer ISR. Set sampling rate to 44100
        // add_repeating_timer_us((int64_t)((1.0/44100.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer);
        // if(alarm_pool_add_repeating_timer_us(core1_alarm_pool, (int64_t)((1.0/11025.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer) != false){
        //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioModule: No timer slots available, could not start audio callback!"));
        // }
        // add_repeating_timer_us((int64_t)((1.0/22050.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer);

        if(add_repeating_timer_us((int64_t)((1.0/11025.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer) == false){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioModule: No timer slots available, could not start audio callback!"));
        }
    #endif
}

size_t stack_size = 4096*2;

void engine_audio_setup(){
    // Fill channels array with channels. This has to be done
    // before any callbacks try to access the channels array
    // (otherwise it would be trying to access all NULLs)
    // Setup should be run once per lifetime, shouldn't need a mutex
    for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        MP_STATE_PORT(channels[icx]) = audio_channel_class_new(&audio_channel_class_type, 0, 0, NULL);
    }

    #if defined(__unix__)
        // Nothing to do
    #elif defined(__arm__)
        // Setup amplifier but make sure it is disabled while PWM is being setup
        gpio_init(26);
        gpio_set_dir(26, GPIO_OUT);
        gpio_put(26, 0);

        // Setup PWM audio pin, bit-depth, and frequency. Duty cycle
        // is only adjusted PWM parameter as samples are retrievd from
        // channel sources
        uint audio_pwm_pin_slice = pwm_gpio_to_slice_num(23);
        gpio_set_function(23, GPIO_FUNC_PWM);
        pwm_config audio_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_pwm_pin_config, 1);
        pwm_config_set_wrap(&audio_pwm_pin_config, 512);   // 125MHz / 1024 = 122kHz
        pwm_init(audio_pwm_pin_slice, &audio_pwm_pin_config, true);

        // Now allow sound to play by enabling the amplifier
        gpio_put(26, 1);

        // Launch timer setup on other core. All audio playback
        // is done on the other core. This means that access
        // to channels and sources will need to be protected...
        // https://github.com/raspberrypi/pico-examples/blob/master/multicore/hello_multicore/multicore.c
        // multicore_reset_core1();
        // multicore_launch_core1(engine_audio_setup_playback);
        engine_audio_setup_playback();
        // if(add_repeating_timer_us((int64_t)((1.0/11025.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer) == false){
        //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioModule: No timer slots available, could not audio callback!"));
        // }
    #endif
}


STATIC mp_obj_t engine_audio_play(mp_obj_t sound_source_obj, mp_obj_t channel_index_obj, mp_obj_t looping_obj){
    // Should probably make sure this doesn't interfere with DMA or interrupt: TODO
    uint8_t channel_index = mp_obj_get_int(channel_index_obj);
    audio_channel_class_obj_t *channel = MP_STATE_PORT(channels[channel_index]);
    channel->source = sound_source_obj;
    channel->looping = mp_obj_get_int(looping_obj);
    return MP_OBJ_FROM_PTR(channel);
}
MP_DEFINE_CONST_FUN_OBJ_3(engine_audio_play_obj, engine_audio_play);


// Set new master volume but clamped to 0.0 ~ 1.0 (no boosting allowed)
STATIC mp_obj_t engine_audio_set_volume(mp_obj_t new_volume){
    master_volume = engine_math_clamp(mp_obj_get_float(new_volume), 0.0f, 1.0f);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_audio_set_volume_obj, engine_audio_set_volume);


STATIC mp_obj_t engine_audio_get_volume(){
    return mp_obj_new_float(master_volume);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_audio_get_volume_obj, engine_audio_get_volume);


// Module attributes
STATIC const mp_rom_map_elem_t engine_audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AudioChannel), (mp_obj_t)&audio_channel_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play), (mp_obj_t)&engine_audio_play_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_volume), (mp_obj_t)&engine_audio_set_volume_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_volume), (mp_obj_t)&engine_audio_get_volume_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_audio_globals, engine_audio_globals_table);

const mp_obj_module_t engine_audio_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_audio_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_audio, engine_audio_user_cmodule);