#include "py/obj.h"
#include "py/mpthread.h"
#include "engine_audio_module.h"
#include "resources/engine_sound_resource_base.h"
#include "resources/engine_wave_sound_resource.h"
#include "resources/engine_tone_sound_resource.h"
#include "resources/engine_rtttl_sound_resource.h"
#include "debug/debug_print.h"
#include <stdlib.h>
#include <string.h>
#include "math/engine_math.h"
#include "utility/engine_time.h"
#include "utility/engine_defines.h"
#include "engine_main.h"



// 4 audio channels. Since audio is large it has to stay in
// flash, but flash is slow to get data from which is a problem
// when sample retrieval time/latency needs to be small to keep
// on track at the relatively high sample rate good audio requires.
// DMA is used to copy data into one of the dual buffer pairs
// each audio channel gets but there's only 12 channels on RP2040
// and one is used for the screen
volatile mp_obj_t channels[CHANNEL_COUNT];

// These scale the amplitudes of each audio sample
// Apart from these, there is also each channel's gain
volatile float master_volume = 1.0f;    // Set by settings file, games cannot set this and effects all audio
volatile float game_volume = 1.0f;      // Games are allowed to set this through `set_volume`


void engine_audio_apply_master_volume(float volume){
    master_volume = volume;
}


float engine_audio_get_master_volume(){
    return master_volume;
}


#if defined(__EMSCRIPTEN__)
    // Nothing to do
#elif defined(__unix__)
    #include <SDL2/SDL.h>
    SDL_AudioSpec audio;
#elif defined(__arm__)
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/structs/xip_ctrl.h"
    #include "hardware/pwm.h"
    #include "hardware/adc.h"
    #include "hardware/timer.h"
    #include "pico/multicore.h"
    #include "io/engine_io_rp3.h"

    // Pin for PWM audio sample wrap callback (faster than repeating timer, by a lot)
    uint audio_callback_pwm_pin_slice;
    pwm_config audio_callback_pwm_pin_config;

    uint8_t *current_source_data = NULL;

    void ENGINE_FAST_FUNCTION(engine_audio_handle_buffer)(audio_channel_class_obj_t *channel, bool *complete){
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
                    *complete = true;
                }else{
                    // Run right away to fill buffer with starting data since looping
                    engine_audio_handle_buffer(channel, complete);
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
                engine_audio_handle_buffer(channel, complete);
            }
        }
    }


    float ENGINE_FAST_FUNCTION(get_wave_sample)(audio_channel_class_obj_t *channel, bool *complete){
        sound_resource_base_class_obj_t *source = channel->source;

        // Keep returning current sample until time to get next one
        if(source->play_counter != 0){
            if(source->play_counter == source->play_counter_max){
                source->play_counter = 0;
            }else{
                source->play_counter++;
                return source->last_sample;
            }
        }

        // Fill buffer with data whether first time or looping
        engine_audio_handle_buffer(channel, complete);

        uint8_t buffer_index = channel->reading_buffer_index;
        uint16_t buffer_byte_offset = channel->buffers_byte_offsets[buffer_index];

        // Add samples to total
        switch(source->bytes_per_sample){
            case 1:
            {
                uint8_t sample_byte = channel->buffers[buffer_index][buffer_byte_offset];       // Get sample as unsigned 8-bit value from 0 to 255
                source->last_sample = (int8_t)(sample_byte - 128);                              // Center sample in preparation for scaling to -1.0 ~ 1.0. Subtract 128 so that 0 -> -128 and 255 -> 127
                source->last_sample = source->last_sample / (float)INT8_MAX;                    // Scale from -128 ~ 127 to -1.0078 ~ 1.0 (will clamp later)
            }
            break;
            case 2:
            {
                uint8_t sample_byte_lsb = channel->buffers[buffer_index][buffer_byte_offset];   // Get the right-most 8-bits as unsigned 8-bit (really is signed 16-bit byte, bits will still exist in same pattern)
                uint8_t sample_byte_msb = channel->buffers[buffer_index][buffer_byte_offset+1]; // Get the left-most 8-bits as unsigned 8-bit (really is signed 16-bit byte, bits will still exist in same pattern)
                source->last_sample = (int16_t)((sample_byte_msb << 8) + sample_byte_lsb);      // Combine bytes to make signed 16-bit value from -32768 ~ 32767
                source->last_sample = source->last_sample / (float)INT16_MAX;                   // Scale from -32768 ~ 32767 to -1.000031 ~ 1.0 (will clamp later)
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


    float ENGINE_FAST_FUNCTION(get_rtttl_sample)(audio_channel_class_obj_t *channel, bool *complete){
        return rtttl_sound_resource_get_sample(channel->source, complete);
    }


    // Samples each channel, adds, normalizes, and sets PWM
    void ENGINE_FAST_FUNCTION(repeating_audio_callback)(void){
        float total_sample = 0;
        bool play_sample = false;

        for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
            bool complete = false;
            audio_channel_class_obj_t *channel = channels[icx];

            if(channel->source == NULL || channel->busy){
                continue;
            }

            // Playing at least one sample, switch flag
            play_sample = true;
            float sample = 0.0f;

            if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
                sample = get_wave_sample(channel, &complete);
            }else if(mp_obj_is_type(channel->source, &tone_sound_resource_class_type)){
                sample = get_tone_sample(channel);
            }else if(mp_obj_is_type(channel->source, &rtttl_sound_resource_class_type)){
                sample = get_rtttl_sample(channel, &complete);
            }

            // Set the amplitude just retrieved as the last sample to have been played on the channel
            channel->amplitude = sample;

            total_sample += sample * channel->gain * game_volume * master_volume;

            if(complete && channel->loop == false){
                audio_channel_stop(channel);
            }
        }

        if(play_sample){
            // Up to the user to make sure all playing channels do not add up and
            // go out of -1.0 ~ 1.0 range. Clamp the total sample sum since
            // very likely it could end of out of bounds, and map to PWM levels
            // NOTE: Set PWM wrap to 512 levels so it will use values from 0 to 511
            total_sample = engine_math_map_clamp(total_sample, -1.0f, 1.0f, 0.0f, 511.0f);

            // Actually set the wrap value to play this sample
            pwm_set_gpio_level(AUDIO_PWM_PIN, (uint32_t)(total_sample));
        }

        pwm_clear_irq(audio_callback_pwm_pin_slice);

        return;
    }
#endif


void engine_audio_setup_playback(){
    #if defined(__EMSCRIPTEN__)
        // Nothing to do
    #elif defined(__unix__)
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
        pwm_config_set_wrap(&audio_pwm_pin_config, 512);   // 150MHz / 512 = 292kHz
        pwm_init(audio_pwm_pin_slice, &audio_pwm_pin_config, true);

        // Now allow sound to play by enabling the amplifier
        gpio_put(AUDIO_ENABLE_PIN, 1);
    #endif
}


void engine_audio_adjust_playback_with_freq(uint32_t core_clock_hz){
    #if defined(__arm__)
        pwm_config_set_wrap(&audio_callback_pwm_pin_config, (uint16_t)((float)(core_clock_hz) / ENGINE_AUDIO_SAMPLE_RATE) - 1);
    #endif
}


void engine_audio_setup(){
    ENGINE_PRINTF("EngineAudio: Setting up...\n");

    // Fill channels array with channels. This has to be done
    // before any callbacks try to access the channels array
    // (otherwise it would be trying to access all NULLs)
    // Setup should be run once per lifetime
    for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        audio_channel_class_obj_t *channel = audio_channel_class_new(&audio_channel_class_type, 0, 0, NULL);
        channels[icx] = channel;
    }

    #if defined(__EMSCRIPTEN__)
        // Nothing to do
    #elif defined(__unix__)
        audio.freq = 22050;
        audio.format = AUDIO_U16;
    #elif defined(__arm__)
        //generate the interrupt at the audio sample rate to set the PWM duty cycle
        audio_callback_pwm_pin_slice = pwm_gpio_to_slice_num(AUDIO_CALLBACK_PWM_PIN);
        pwm_clear_irq(audio_callback_pwm_pin_slice);
        pwm_set_irq_enabled(audio_callback_pwm_pin_slice, true);
        irq_set_exclusive_handler(PWM_IRQ_WRAP_0, repeating_audio_callback);
        irq_set_priority(PWM_IRQ_WRAP_0, 1);
        irq_set_enabled(PWM_IRQ_WRAP_0, true);
        audio_callback_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_callback_pwm_pin_config, 1);
        engine_audio_adjust_playback_with_freq(150 * 1000 * 1000);
        pwm_init(audio_callback_pwm_pin_slice, &audio_callback_pwm_pin_config, true);
    #endif
}


void engine_audio_reset(){
    ENGINE_INFO_PRINTF("EngineAudio: Stopping all channels...");

    // By default, set the game volume back to max
    // (the master volume can still scale this lower)
    game_volume = 1.0f;

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

    if(mp_obj_is_type(sound_resource_obj, &wave_sound_resource_class_type)){
        sound_resource_base_class_obj_t *source = sound_resource_obj;

        // Very important to set this link! The source needs access to the channel that
        // is playing it (if one is) so that it can remove itself from the linked channel's
        // source
        source->channel = channel;
    }else if(mp_obj_is_type(sound_resource_obj, &tone_sound_resource_class_type)){
        tone_sound_resource_class_obj_t *source = sound_resource_obj;

        // Very important to set this link! The source needs access to the channel that
        // is playing it (if one is) so that it can remove itself from the linked channel's
        // source
        source->channel = channel;
    }else if(mp_obj_is_type(sound_resource_obj, &rtttl_sound_resource_class_type)){
        rtttl_sound_resource_class_obj_t *source = sound_resource_obj;

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
    ID: audio_stop
    DESC: Stops playing audio on channel at index
    PARAM: [type=int]       [name=channel_index]  [value=0 ~ 3]
    RETURN: None
*/
static mp_obj_t engine_audio_stop(mp_obj_t channel_index_obj){
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
    ID: audio_play
    DESC: Starts playing an audio source on a given channel and looping or not. It is up to the user to change the gains of the returned channels so that the audio does not clip.
    PARAM: [type=object]    [name=sound_resource] [value={ref_link:WaveSoundResource} or {ref_link:ToneSoundResource}]
    PARAM: [type=int]       [name=channel_index]  [value=0 ~ 3]
    PARAM: [type=boolean]   [name=loop]           [value=True or False]
    RETURN: {ref_link:AudioChannel}
*/
static mp_obj_t engine_audio_play(mp_obj_t sound_resource_obj, mp_obj_t channel_index_obj, mp_obj_t loop_obj){
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
    ID: set_volume
    DESC: Sets the game volume clamped between 0.0 and 1.0
    PARAM: [type=float] [name=set_volume] [value=any]
    RETURN: None
*/
static mp_obj_t engine_audio_set_volume(mp_obj_t new_volume){
    // Don't clamp so that users can clip their waveforms
    // which adds more area under the curve and therefore
    // is louder (sounds worse though)
    game_volume = mp_obj_get_float(new_volume);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_audio_set_volume_obj, engine_audio_set_volume);


/*  --- doc ---
    NAME: get_volume
    ID: get_volume
    DESC: Returns the currently set game volume between 0.0 and 1.0
    RETURN: None
*/
static mp_obj_t engine_audio_get_volume(){
    return mp_obj_new_float(master_volume);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_audio_get_volume_obj, engine_audio_get_volume);


static mp_obj_t engine_audio_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_audio_module_init_obj, engine_audio_module_init);


/*  --- doc ---
    NAME: engine_audio
    ID: engine_audio
    DESC: Module for controlling/playing audio through four channels.
    ATTR: [type=object]     [name={ref_link:AudioChannel}]  [value=function]
    ATTR: [type=function]   [name={ref_link:audio_play}]    [value=function]
    ATTR: [type=function]   [name={ref_link:audio_stop}]    [value=function]
    ATTR: [type=function]   [name={ref_link:set_volume}]    [value=function]
    ATTR: [type=function]   [name={ref_link:get_volume}]    [value=function]
*/
static const mp_rom_map_elem_t engine_audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_audio) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_audio_module_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_AudioChannel), (mp_obj_t)&audio_channel_class_type },
    { MP_OBJ_NEW_QSTR(MP_QSTR_play), (mp_obj_t)&engine_audio_play_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_stop), (mp_obj_t)&engine_audio_stop_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_volume), (mp_obj_t)&engine_audio_set_volume_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_get_volume), (mp_obj_t)&engine_audio_get_volume_obj },
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_audio_globals, engine_audio_globals_table);

const mp_obj_module_t engine_audio_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_audio_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_audio, engine_audio_user_cmodule);