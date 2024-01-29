#include "py/obj.h"
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
// and one is used for the screen
audio_channel_class_obj_t *channels[CHANNEL_COUNT];



#if defined(__unix__)
    // Nothing to do
#elif defined(__arm__)
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/pwm.h"

    // pg. 542: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
    // https://github.com/raspberrypi/pico-examples/blob/master/timer/hello_timer/hello_timer.c#L11-L57
    // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#rpipdb65a0bdce0635d95877
    // https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#interrupt-numbers
    struct repeating_timer repeating_audio_timer;


    void engine_audio_handle_buffer(audio_channel_class_obj_t *channel){
        // When 'buffer_byte_offset = 0' that means the buffer hasn't been filled before, fill it (see that after this function it is immediately incremented)
        // When 'buffer_byte_offset >= CHANNEL_BUFFER_SIZE' that means the index has run out of data, fill it with more
        if(channel->buffer_byte_offset == 0 || channel->buffer_byte_offset >= CHANNEL_BUFFER_SIZE){
            // Reset for the second case above
            channel->buffer_byte_offset = 0;

            // Using the sound resource base, fill this channel's
            // buffer with audio data from the source resource
            uint32_t filled_amount = channel->source->fill_buffer(channel->source, channel->buffer, channel->source_byte_offset, CHANNEL_BUFFER_SIZE);

            // Filled amount will always be equal to or less than to 
            // 0 the 'size' passed to 'fill_buffer'. In the case it was
            // with filled with something, increment to the amount filled
            // further. In the case it is filled with nothing, that means
            // the last fill made us reach the end of the source data,
            // figure out if this channel should stop or loop. If loop,
            // run again right away to fill with more data after resetting
            // 'source_byte_offset' 
            if(filled_amount > 0){
                channel->source_byte_offset += filled_amount;
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
    bool repeating_audio_callback(struct repeating_timer *t){
        float sample = 0;

        for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
            // Go over every channel and check if set to something usable
            if(channels[icx]->source != NULL){
                // Fill buffer with data whether first time or looping
                engine_audio_handle_buffer(channels[icx]);

                // TODO: Mix samples!
                switch(channels[icx]->source->bytes_per_sample){
                    case 1:
                    {
                        sample = (float)channels[icx]->buffer[channels[icx]->buffer_byte_offset];   // Get 8-bit sample
                        sample = sample / (float)UINT8_MAX;                                         // Scale from 0 ~ 255 to 0.0 ~ 1.0
                        sample = sample * channels[icx]->volume;                                    // Scale sample by channel volume
                        sample = (engine_math_clamp(sample, 0.0f, 1.0f) * 512.0f);                  // Clamp volume scaled sample and scale from 0.0 ~ 1.0 to 0.0 to 512.0
                    }
                    break;
                    case 2:
                    {   
                        int32_t temp = 0;
                        int32_t b0 = channels[icx]->buffer[channels[icx]->buffer_byte_offset+1];
                        int32_t b1 = channels[icx]->buffer[channels[icx]->buffer_byte_offset];
                        temp = temp | (b0 << 8);
                        temp = temp | (b1);
                        sample = temp;                                              // Get 16-bit sample
                        sample = sample / (float)UINT16_MAX;                        // Scale from 0 ~ 65535 to 0.0 ~ 1.0
                        sample = sample * channels[icx]->volume;                    // Scale sample by channel volume
                        sample = (engine_math_clamp(sample, 0.0f, 1.0f) * 512.0f);  // Clamp volume scaled sample and scale from 0.0 ~ 1.0 to 0.0 to 512.0
                    }
                    break;
                    default:
                        ENGINE_ERROR_PRINTF("Audio source with %d bytes per sample is not supported!", channels[icx]->source->bytes_per_sample);
                }

                // Make sure to grab the next sample next time
                channels[icx]->buffer_byte_offset += channels[icx]->source->bytes_per_sample;
            }
            
            // TODO, update channel time based on 'source_byte_offset' and source 'total_sample_count'
        }
        
        // TODO, compress samples and output duty cycle
        pwm_set_gpio_level(23, (uint32_t)sample);
        return true;
    }
#endif


void engine_audio_setup(){
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

        // Setup timer ISR. Set sampling rate to 44100
        // add_repeating_timer_us((int64_t)((1.0/44100.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer);
        if(add_repeating_timer_us((int64_t)((1.0/11025.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer) == false){
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioModule: No timer slots available, could not audio callback!"));
        }
        // add_repeating_timer_us((int64_t)((1.0/22050.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer);
    #endif

    // Fill channels array with channels
    for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        channels[icx] = audio_channel_class_new(&audio_channel_class_type, 0, 0, NULL);
    }
}


STATIC mp_obj_t engine_audio_play(mp_obj_t sound_source_obj, mp_obj_t channel_index_obj, mp_obj_t looping_obj){
    // Should probably make sure this doesn't interfere with DMA or interrupt: TODO
    uint8_t channel_index = mp_obj_get_int(channel_index_obj);
    channels[channel_index]->source = sound_source_obj;
    channels[channel_index]->looping = mp_obj_get_int(looping_obj);
    return MP_OBJ_FROM_PTR(channels[channel_index]);
}
MP_DEFINE_CONST_FUN_OBJ_3(engine_audio_play_obj, engine_audio_play);


// Module attributes
STATIC const mp_rom_map_elem_t engine_audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AudioChannel), (mp_obj_t)&audio_channel_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play), (mp_obj_t)&engine_audio_play_obj },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_audio_globals, engine_audio_globals_table);

const mp_obj_module_t engine_audio_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_audio_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_audio, engine_audio_user_cmodule);