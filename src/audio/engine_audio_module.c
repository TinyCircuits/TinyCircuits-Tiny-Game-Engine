#include "py/obj.h"
#include "py/mpthread.h"
#include "engine_audio_module.h"
#include "resources/engine_sound_resource_base.h"
#include "resources/engine_wave_sound_resource.h"
#include "resources/engine_tone_sound_resource.h"
#include "debug/debug_print.h"
#include <stdlib.h>
#include <string.h>
#include "math/engine_math.h"
#include "utility/engine_time.h"
#include "utility/engine_defines.h"



// 4 audio channels. Since audio is large it has to stay in
// flash, but flash is slow to get data from which is a problem
// when sample retrieval time/latency needs to be small to keep
// on track at the relatively high sample rate good audio requires.
// DMA is used to copy data into one of the dual buffer pairs
// each audio channel gets but there's only 12 channels on RP2040
// and one is used for the screen
volatile mp_obj_t channels[CHANNEL_COUNT];

// The master volume that all mixed samples are scaled by (0.0 ~ 1.0)
volatile float master_volume = 1.0f;


#if defined(__unix__)
    // Nothing to do
#elif defined(__arm__)
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/structs/xip_ctrl.h"
    #include "hardware/pwm.h"
    #include "hardware/timer.h"
    #include "pico/multicore.h"

    #define AUDIO_PWM_PIN 23
    #define AUDIO_CALLBACK_PWM_PIN 24
    #define AUDIO_ENABLE_PIN 20

    // Pin for PWM audio sample wrap callback (faster than repeating timer, by a lot)
    uint audio_callback_pwm_pin_slice;

    uint8_t *current_source_data = NULL;

    void ENGINE_FAST_FUNCTION(engine_audio_handle_buffer)(audio_channel_class_obj_t *channel){
        // When 'buffer_byte_offset = 0' that means the buffer hasn't been filled before, fill it (see that after this function it is immediately incremented)
        // When 'buffer_byte_offset >= channel->buffer_end' that means the index has run out of data, fill it with more
        if(channel->buffers_byte_offsets[channel->reading_buffer_index] == 0 || channel->buffers_byte_offsets[channel->reading_buffer_index] >= channel->buffers_ends[channel->reading_buffer_index]){
            // Reset for the second case above
            channel->buffers_byte_offsets[channel->reading_buffer_index] = 0;

            // Using the sound resource base, fill this channel's
            // buffer with audio data from the source resource
            // channel->buffer_end = channel->source->fill_buffer(channel->source, channel->buffer, channel->source_byte_offset, CHANNEL_BUFFER_SIZE);

            if(channel->source == NULL){
                return;
            }

            sound_resource_base_class_obj_t *source = channel->source;

            current_source_data = source->get_data(channel, CHANNEL_BUFFER_SIZE, &channel->buffers_ends[channel->reading_buffer_index]);

            // memcpy((uint8_t*)channel->buffer, (uint8_t*)current_source_data, channel->buffer_end);

            // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/flash/xip_stream/flash_xip_stream.c#L45-L48
            // while (!(xip_ctrl_hw->stat & XIP_STAT_FIFO_EMPTY))
            //     (void) xip_ctrl_hw->stream_fifo;
            // xip_ctrl_hw->stream_addr = (uint32_t)current_source_data;
            // xip_ctrl_hw->stream_ctr = channel->buffer_end;

            // Just in case we were too quick, wait while previous DMA might still be active
            if(dma_channel_is_busy(channel->dma_channel)){
                ENGINE_WARNING_PRINTF("AudioModule: Waiting on previous DMA transfer to complete, this ideally shouldn't happen");
                dma_channel_wait_for_finish_blocking(channel->dma_channel);
            }

            channel->reading_buffer_index = 1 - channel->reading_buffer_index;
            channel->filling_buffer_index = 1 - channel->filling_buffer_index;

            // https://github.com/raspberrypi/pico-examples/blob/master/flash/xip_stream/flash_xip_stream.c#L58-L70
            dma_channel_configure(
                channel->dma_channel,                                   // Channel to be configured
                &channel->dma_config,                                   // The configuration we just created
                channel->buffers[channel->filling_buffer_index],        // The initial write address
                current_source_data,                                    // The initial read address
                channel->buffers_ends[channel->filling_buffer_index],   // Number of transfers; in this case each is 1 byte
                true                                                    // Start immediately
            );

            // Filled amount will always be equal to or less than to 
            // 0 the 'size' passed to 'fill_buffer'. In the case it was
            // filled with something, increment to the amount filled
            // further. In the case it is filled with nothing, that means
            // the last fill made us reach the end of the source data,
            // figure out if this channel should stop or loop. If loop,
            // run again right away to fill with more data after resetting
            // 'source_byte_offset' 
            if(channel->buffers_ends[channel->filling_buffer_index] > 0){
                channel->source_byte_offset += channel->buffers_ends[channel->filling_buffer_index];
            }else{
                // Gets reset no matter what, whether looping or not
                channel->source_byte_offset = 0;

                // If not looping, disable/remove the source and stop this
                // channel from being played, otherwise, fill with start data
                if(channel->loop == false){
                    audio_channel_stop(channel);
                }else{
                    // Run right away to fill buffer with starting data since looping
                    engine_audio_handle_buffer(channel);
                }
            }

            // Not the best solution but when these are both 1 that means
            // no data has been loaded yet, block until data is loaded into
            // 1 then load the other buffer while not blocking so that it
            // can be switched to next time for reading
            if(channel->reading_buffer_index == channel->filling_buffer_index){
                dma_channel_wait_for_finish_blocking(channel->dma_channel);

                // Flip only the buffer to fill since we're going to fill it now
                channel->filling_buffer_index = 1 - channel->filling_buffer_index;

                // Fill the other buffer right now
                engine_audio_handle_buffer(channel);
            }
        }
    }


    float ENGINE_FAST_FUNCTION(get_wave_sample)(audio_channel_class_obj_t *channel){
        sound_resource_base_class_obj_t *source = channel->source;

        // Depending on engine_playback sample rate
        if(source->play_counter != 0){
            if(source->sample_rate == 11025 && source->play_counter == 2){
                source->play_counter = 0;
            }else if(source->sample_rate == 22050 && source->play_counter == 1){
                source->play_counter = 0;
            }
            // else if(source->sample_rate == 44100 && source->play_counter == 1){
            //     source->play_counter = 0;
            // }
            else{
                source->play_counter++;
                return source->last_sample;
            }
        }

        // Fill buffer with data whether first time or looping
        engine_audio_handle_buffer(channel);

        uint8_t buffer_index = channel->reading_buffer_index;
        uint16_t buffer_byte_offset = channel->buffers_byte_offsets[buffer_index];

        // Add samples to total
        switch(source->bytes_per_sample){
            case 1:
            {
                uint8_t sample_byte = channel->buffers[buffer_index][buffer_byte_offset];       // Get sample as unsigned 8-bit value from 0 to 255
                source->last_sample = (int8_t)(sample_byte - 128);                              // Center sample in preparation for scaling to -1.0 ~ 1.0. Subtract 128 so that 0 -> -128 and 255 -> 127
                source->last_sample = source->last_sample / (float)INT8_MAX;                    // Scale from -128 ~ 127 to -1.0078 ~ 1.0 (will clamp later)
                source->last_sample = source->last_sample * channel->gain * master_volume;      // Scale sample by channel gain and master volume (that can be 0.0 ~ 1.0 each)
                source->last_sample = (engine_math_clamp(source->last_sample, -1.0f, 1.0f));    // Clamp volume scaled sample to -1.0 ~ 1.0
            }
            break;
            case 2:
            {   
                uint8_t sample_byte_lsb = channel->buffers[buffer_index][buffer_byte_offset];   // Get the right-most 8-bits as unsigned 8-bit (really is signed 16-bit byte, bits will still exist in same pattern)
                uint8_t sample_byte_msb = channel->buffers[buffer_index][buffer_byte_offset+1]; // Get the left-most 8-bits as unsigned 8-bit (really is signed 16-bit byte, bits will still exist in same pattern)
                source->last_sample = (int16_t)((sample_byte_msb << 8) + sample_byte_lsb);      // Combine bytes to make signed 16-bit value from -32768 ~ 32767
                source->last_sample = source->last_sample / (float)INT16_MAX;                   // Scale from -32768 ~ 32767 to -1.000031 ~ 1.0 (will clamp later)
                source->last_sample = source->last_sample * channel->gain * master_volume;      // Scale sample by channel gain and master volume (that can be 0.0 ~ 1.0 each)
                source->last_sample = engine_math_clamp(source->last_sample, -1.0f, 1.0f);      // Clamp volume scaled sample to -1.0 ~ 1.0
            }
            break;
            default:
                ENGINE_ERROR_PRINTF("AudioModule: Audio source with %d bytes per sample is not supported!", source->bytes_per_sample);
        }

        // Set for the next sample
        channel->buffers_byte_offsets[buffer_index] += source->bytes_per_sample;

        // Calculate the current time that we're at in the channel's source
        channel->time = (1.0f / source->sample_rate) * (channel->source_byte_offset / source->bytes_per_sample);

        source->play_counter++;

        return source->last_sample;
    }


    float ENGINE_FAST_FUNCTION(get_tone_sample)(audio_channel_class_obj_t *channel){
        return tone_sound_resource_get_sample(channel->source);
    }


    // Samples each channel, adds, normalizes, and sets PWM
    void ENGINE_FAST_FUNCTION(repeating_audio_callback)(void){
        float total_sample = 0;
        bool play_sample = false;

        for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){

            audio_channel_class_obj_t *channel = channels[icx];

            if(channel->source == NULL || channel->busy){
                continue;
            }

            // Playing at least one sample, switch flag
            play_sample = true;

            if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
                total_sample += get_wave_sample(channel);
            }else{
                total_sample += get_tone_sample(channel);
            }
        }
        
        if(play_sample){
            // Up to the user to make sure all playing channels do not add up and
            // go out of -1.0 ~ 1.0 range. First clamp the total sample sum since
            // very likely it could end of out of bounds and then map to PWM levels
            // NOTE: Set PWM wrap to 512 levels so it will use values from 0 to 511
            total_sample = engine_math_clamp(total_sample, -1.0f, 1.0f);
            total_sample = engine_math_map(total_sample, -1.0f, 1.0f, 0.0f, 511.0f);

            // total_sample = engine_math_map(sinf(millis() * 8.0f), -1.0f, 1.0f, 0.0f, 511.0f);

            // Actually set the wrap value to play this sample
            pwm_set_gpio_level(AUDIO_PWM_PIN, (uint32_t)(total_sample));
        }

        pwm_clear_irq(audio_callback_pwm_pin_slice);
        return;
    }
#endif


void engine_audio_setup_playback(){
    #if defined(__unix__)
        // Nothing to do
    #elif defined(__arm__)
        // Setup amplifier but make sure it is disabled while PWM is being setup
        gpio_init(AUDIO_ENABLE_PIN);
        gpio_set_dir(AUDIO_ENABLE_PIN, GPIO_OUT);
        gpio_put(AUDIO_ENABLE_PIN, 0);

        // Setup PWM audio pin, bit-depth, and frequency. Duty cycle
        // is only adjusted PWM parameter as samples are retrievd from
        // channel sources
        uint audio_pwm_pin_slice = pwm_gpio_to_slice_num(AUDIO_PWM_PIN);
        gpio_set_function(AUDIO_PWM_PIN, GPIO_FUNC_PWM);
        pwm_config audio_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_pwm_pin_config, 1);
        pwm_config_set_wrap(&audio_pwm_pin_config, 512);   // 125MHz / 1024 = 122kHz
        pwm_init(audio_pwm_pin_slice, &audio_pwm_pin_config, true);

        // Now allow sound to play by enabling the amplifier
        gpio_put(AUDIO_ENABLE_PIN, 1);
    #endif
}


void engine_audio_setup(){
    ENGINE_FORCE_PRINTF("EngineAudio: Setting up...");

    // Fill channels array with channels. This has to be done
    // before any callbacks try to access the channels array
    // (otherwise it would be trying to access all NULLs)
    // Setup should be run once per lifetime
    for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        audio_channel_class_obj_t *channel = audio_channel_class_new(&audio_channel_class_type, 0, 0, NULL);
        channels[icx] = channel;
    }

    #if defined(__unix__)
        // Nothing to do
    #elif defined(__arm__)
        // if(add_repeating_timer_us((int64_t)((1.0/ENGINE_AUDIO_SAMPLE_RATE) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer) == false){
        //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioModule: No timer slots available, could not audio callback!"));
        // }

        //generate the interrupt at the audio sample rate to set the PWM duty cycle
        audio_callback_pwm_pin_slice = pwm_gpio_to_slice_num(AUDIO_CALLBACK_PWM_PIN);
        pwm_clear_irq(audio_callback_pwm_pin_slice);
        pwm_set_irq_enabled(audio_callback_pwm_pin_slice, true);
        irq_set_exclusive_handler(PWM_IRQ_WRAP_0, repeating_audio_callback);
        irq_set_priority(PWM_IRQ_WRAP_0, 1);
        irq_set_enabled(PWM_IRQ_WRAP_0, true);
        pwm_config audio_callback_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_callback_pwm_pin_config, 1);
        pwm_config_set_wrap(&audio_callback_pwm_pin_config, ((125.0f * 1000.0f * 1000.0f) / ENGINE_AUDIO_SAMPLE_RATE) - 1);
        pwm_init(audio_callback_pwm_pin_slice, &audio_callback_pwm_pin_config, true);
    #endif
}


void engine_audio_stop_all(){
    ENGINE_INFO_PRINTF('EngineAudio: Stopping all channels...');

    for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        // Check that each channel is not NULL since reset
        // can be called before hardware init
        if(channels[icx] != NULL){
            audio_channel_stop(channels[icx]);
        }
    }
}


void engine_audio_play_on_channel(mp_obj_t sound_resource_obj, audio_channel_class_obj_t *channel, mp_obj_t loop_obj){
    // Before anything, make sure to stop the channel
    // incase of two `.play(...)` calls in a row
    audio_channel_stop(channel);

    // Mark the channel as busy so that the interrupt
    // doesn't use it
    channel->busy = true;

    if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
        sound_resource_base_class_obj_t *source = sound_resource_obj;

        // Very important to set this link! The source needs access to the channel that
        // is playing it (if one is) so that it can remove itself from the linked channel's
        // source
        source->channel = channel;
    }else{
        tone_sound_resource_class_obj_t *source = sound_resource_obj;

        // Very important to set this link! The source needs access to the channel that
        // is playing it (if one is) so that it can remove itself from the linked channel's
        // source
        source->channel = channel;
    }

    channel->source = sound_resource_obj;
    channel->loop = mp_obj_get_int(loop_obj);
    channel->done = false;

    // Now let the interrupt use it
    channel->busy = false;
}


/*  --- doc ---
    NAME: stop
    DESC: Stops playing audio on channel at index
    PARAM: [type=int]       [name=channel_index]  [value=0 ~ 3]                                                                                                          
    RETURN: None
*/ 
STATIC mp_obj_t engine_audio_stop(mp_obj_t channel_index_obj){
    uint8_t channel_index = mp_obj_get_int(channel_index_obj);

    if(channel_index > 3){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioModule: ERROR: Tried to stop audio channel using an index that does not exist"));
    }

    audio_channel_stop(channels[channel_index]);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_audio_stop_obj, engine_audio_stop);


/*  --- doc ---
    NAME: play
    DESC: Starts playing an audio source on a given channel and looping or not. It is up to the user to change the gains of the returned channels so that the audio does not clip.
    PARAM: [type=object]    [name=sound_resource] [value={ref_link:WaveSoundResource}]
    PARAM: [type=int]       [name=channel_index]  [value=0 ~ 3]                                                          
    PARAM: [type=boolean]   [name=loop]           [value=True or False]                                                  
    RETURN: {ref_link:AudioChannel}
*/ 
STATIC mp_obj_t engine_audio_play(mp_obj_t sound_resource_obj, mp_obj_t channel_index_obj, mp_obj_t loop_obj){
    // Should probably make sure this doesn't
    // interfere with DMA or interrupt: TODO
    uint8_t channel_index = mp_obj_get_int(channel_index_obj);
    audio_channel_class_obj_t *channel = channels[channel_index];

    engine_audio_play_on_channel(sound_resource_obj, channel, loop_obj);

    return MP_OBJ_FROM_PTR(channel);
}
MP_DEFINE_CONST_FUN_OBJ_3(engine_audio_play_obj, engine_audio_play);


/*  --- doc ---
    NAME: set_volume
    DESC: Sets the master volume clamped between 0.0 and 1.0. In the future, this will be persistent and stored/restored using a settings file (TODO)
    PARAM: [type=float] [name=set_volume] [value=0.0 ~ 1.0]
    RETURN: None
*/ 
STATIC mp_obj_t engine_audio_set_volume(mp_obj_t new_volume){
    master_volume = engine_math_clamp(mp_obj_get_float(new_volume), 0.0f, 1.0f);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_audio_set_volume_obj, engine_audio_set_volume);


/*  --- doc ---
    NAME: get_volume
    DESC: Returns the currently set master volume between 0.0 and 1.0
    RETURN: None
*/ 
STATIC mp_obj_t engine_audio_get_volume(){
    return mp_obj_new_float(master_volume);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_audio_get_volume_obj, engine_audio_get_volume);


/*  --- doc ---
    NAME: engine_audio
    DESC: Module for controlling/playing audio through four channels.
    ATTR: [type=object]     [name={ref_link:AudioChannel}]  [value=function]
    ATTR: [type=function]   [name={ref_link:play}]          [value=function] 
    ATTR: [type=function]   [name={ref_link:set_volume}]    [value=function]
    ATTR: [type=function]   [name={ref_link:get_volume}]    [value=function]
*/ 
STATIC const mp_rom_map_elem_t engine_audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AudioChannel), (mp_obj_t)&audio_channel_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play), (mp_obj_t)&engine_audio_play_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&engine_audio_stop_obj },
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