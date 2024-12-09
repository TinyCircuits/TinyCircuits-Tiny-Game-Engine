#include "engine_audio_channel.h"
#include "math/engine_math.h"
#include "debug/debug_print.h"
#include "audio/engine_audio_module.h"
#include <stdlib.h>
#include <string.h>

#include "resources/engine_wave_sound_resource.h"
#include "resources/engine_tone_sound_resource.h"
#include "resources/engine_rtttl_sound_resource.h"


#if defined(__EMSCRIPTEN__)
    #include "engine_audio_web.h"
#elif defined(__unix__)
    #include "engine_audio_unix.h"
#elif defined(__arm__)
    #include "engine_audio_rp3.h"
#endif


void audio_channeL_buffer_reset(audio_channel_class_obj_t *channel){
    channel->buffers_data_len[0] = 0;
    channel->buffers_data_len[1] = 0;
    channel->buffers_byte_cursor[0] = 0;
    channel->buffers_byte_cursor[1] = 0;
    channel->buffer_to_fill_index = 0;
    channel->buffer_to_read_index = 1;
    channel->source_byte_cursor = 0;
}


void audio_channel_reset(audio_channel_class_obj_t *channel){
    channel->source = NULL;
    channel->gain = 1.0f;
    channel->time = 0.0f;
    channel->amplitude = 0.0f;
    channel->done = true;
    channel->loop = false;
    audio_channeL_buffer_reset(channel);
}


mp_obj_t audio_channel_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New AudioChannel");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // Each channel lives for as long as the device is on.
    // This was verified to work by disabling the audio interrupts and printing out `print((dir(chan)))` of
    // a channel started by `chan = engine_audio.play(...)`
    audio_channel_class_obj_t *self = (audio_channel_class_obj_t*)malloc(sizeof(audio_channel_class_obj_t));
    self->base.type = &audio_channel_class_type;

    self->buffers[0] = (uint8_t*)malloc(CHANNEL_BUFFER_LEN);   // Use C heap. Easier to avoid gc and we have persistent buffers anyways
    self->buffers[1] = (uint8_t*)malloc(CHANNEL_BUFFER_LEN);   // Use C heap. Easier to avoid gc and we have persistent buffers anyways
    audio_channel_reset(self);

    // Make sure to clear the buffers to zero to avoid static sound
    // when audio first starts playing
    memset(self->buffers[0], 0x0000, sizeof(uint8_t) * CHANNEL_BUFFER_LEN);
    memset(self->buffers[1], 0x0000, sizeof(uint8_t) * CHANNEL_BUFFER_LEN);

    // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/dma/hello_dma/hello_dma.c#L21-L30
    // https://github.com/raspberrypi/pico-examples/blob/master/flash/xip_stream/flash_xip_stream.c#L58-L70 (see pg. 127 of rp2040 datasheet: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
    // Get a free DMA channel and panic if there isn't one,
    // get a default DMA config, and set the transfer size
    // to 8-bits since smallest audio sample bit-depth is 8
    // (using 16 would mean we'd copy too much data in 8-bit case)
    #if defined(__EMSCRIPTEN__)
        engine_audio_web_channel_init_one_time();
    #elif defined(__unix__)
        engine_audio_unix_channel_init_one_time(self);
    #elif defined(__arm__)
        engine_audio_rp3_channel_init_one_time(&self->dma_copy_channel, &self->dma_copy_config);
    #endif
    
    return MP_OBJ_FROM_PTR(self);
}


// The only reason this entire AudioChannel class exists is so that
// users have an easy way to keep track of channels in MicroPython.
// There are a fixed number of audio channels due to practical sound
// limitations (what's the point of playing 32 sounds on 32 channels at
// the same time?) and constrained resources (dynamic dual buffers)
mp_obj_t audio_channel_class_new_dummy(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: New AudioChannel objects can not be created. These can only constructed internally by the engine"));
    return MP_OBJ_FROM_PTR(mp_const_none);
}


/*  --- doc ---
    NAME: play
    ID: audio_channel_play
    DESC: Starts playing an audio source on a given channel and looping or not. It is up to the user to change the gains of the returned channels so that the audio does not clip.
    PARAM: [type=object]    [name=sound_resource] [value={ref_link:WaveSoundResource}]
    PARAM: [type=boolean]   [name=loop]           [value=True or False] 
    RETURN: None
*/ 
mp_obj_t audio_channel_play(mp_obj_t self_in, mp_obj_t sound_resource_obj, mp_obj_t loop_obj){
    engine_audio_play_on_channel(sound_resource_obj, self_in, loop_obj);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(audio_channel_play_obj, audio_channel_play);


bool audio_channel_fill_internal_buffer(audio_channel_class_obj_t *channel, uint8_t buffer_to_fill_index){
    // By default, assume the source is not
    // indicating it is done providing samples
    bool complete = false;

    uint8_t *channel_buffer_to_fill = channel->buffers[buffer_to_fill_index];
    uint32_t filled_channel_buffer_len = 0;

    if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
        filled_channel_buffer_len = wave_fill_dest(channel->source, channel, channel_buffer_to_fill, CHANNEL_BUFFER_LEN, &complete);
    }else if(mp_obj_is_type(channel->source, &tone_sound_resource_class_type)){
        filled_channel_buffer_len = tone_fill_dest(channel->source, channel, channel_buffer_to_fill, CHANNEL_BUFFER_LEN, &complete);
    }else if(mp_obj_is_type(channel->source, &rtttl_sound_resource_class_type)){
        filled_channel_buffer_len = rtttl_fill_dest(channel->source, channel, channel_buffer_to_fill, CHANNEL_BUFFER_LEN, &complete);
    }

    // Update the channel with how many bytes were copied
    channel->buffers_data_len[buffer_to_fill_index] = filled_channel_buffer_len;
    channel->buffers_byte_cursor[buffer_to_fill_index] = 0;

    return complete;
}


// Fills `output` with `count` float samples
// (may be less than -1.0 or grater than 1.0)
//
// Returns `true` when the channel determines
// it reached the end of the source data and
// looping is not enabled, otherwise, `false`
//
// Here is how data is moved from sources to outputs
//
//   wave_resource (pcm8, pcm16), tone_resource (generated pcm8), rtttl_resource (grabs from tone generated pcm8)
//                                                           |
//                                                           | data from any of these sources are moved in their current raw form to audio channel buffers (DMA from flash/ram to ram)
//                                                           V
//                                                  autio_channel_buffer
//                                                           |
//                                                           | As the playback buffer needs more data, it is converted and volume/gain mixed on the fly for the requested amounts
//                                                           V
//                                                    playback_buffer
uint32_t audio_channel_get_samples(audio_channel_class_obj_t *channel, float *output, uint32_t sample_count, float volume, bool *complete){
    // By default, assume the source is not
    // indicating it is done providing samples
    *complete = false;

    // Check if more samples should be placed into
    // channel audio buffers from source, and do it if
    // the audio from the current read buffer has been
    // filled
    uint8_t  channel_buffer_to_read_index = channel->buffer_to_read_index;
    uint32_t channel_buffer_byte_cursor   = channel->buffers_byte_cursor[channel_buffer_to_read_index];
    uint32_t channel_buffer_data_len      = channel->buffers_data_len[channel_buffer_to_read_index];
    if(channel_buffer_byte_cursor >= channel_buffer_data_len){
        // 1. Both buffers have audio samples but the read buffer
        //    has been completly played, switch buffers
        channel->buffer_to_fill_index = !channel->buffer_to_fill_index; // 0 to 1 or 1 to 0
        channel->buffer_to_read_index = !channel->buffer_to_read_index; // 1 to 0 or 0 to 1

        // 2. Now that the channels have been switched, fill the
        //    fill buffer with new data while the other buffer is
        //    read from
        *complete = audio_channel_fill_internal_buffer(channel, channel->buffer_to_fill_index);

        // When we run through all the data in the source,
        // loop back to the start (decided at a later stage
        // if the channel should keep going, but this needs
        // to happen in any case). Would also be set when
        // new audio is played on this channel
        if(*complete){
            channel->source_byte_cursor = 0;
        }
    }

    // Get the buffer to read from and the cursor inside it + how much data inside it
    channel_buffer_to_read_index     = channel->buffer_to_read_index;
    channel_buffer_byte_cursor       = channel->buffers_byte_cursor[channel_buffer_to_read_index];
    channel_buffer_data_len          = channel->buffers_data_len[channel_buffer_to_read_index];
    uint8_t *channel_buffer_to_read  = channel->buffers[channel_buffer_to_read_index];

    // Calculate how many bytes are left to read in the channel
    uint32_t channel_remaining_bytes = channel_buffer_data_len - channel_buffer_byte_cursor;

    // Convert and copy from audio channel to output. Typically, on hardware,
    // the output will be a single float, but on other platforms it might be
    // a large buffer of floats
    if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
        wave_sound_resource_class_obj_t *wave = channel->source;

        // Calculate number of remaining samples in the channel buffer,
        // and set the number of samples to be converted to whatever is
        // smallest (actually available vs. requested)
        uint16_t channel_remaining_samples = channel_remaining_bytes / wave->bytes_per_sample;
        sample_count = (channel_remaining_samples < sample_count) ? channel_remaining_samples : sample_count;

        channel_buffer_byte_cursor += wave_convert(wave, (channel_buffer_to_read + channel_buffer_byte_cursor), output, sample_count, volume);
    }else if(mp_obj_is_type(channel->source, &tone_sound_resource_class_type)){
        tone_sound_resource_class_obj_t *tone = channel->source;

        channel_buffer_byte_cursor += tone_convert(tone, (channel_buffer_to_read + channel_buffer_byte_cursor), output, sample_count, volume);
    }else if(mp_obj_is_type(channel->source, &rtttl_sound_resource_class_type)){
        rtttl_sound_resource_class_obj_t *rtttl = channel->source;

        channel_buffer_byte_cursor += rtttl_convert(rtttl, (channel_buffer_to_read + channel_buffer_byte_cursor), output, sample_count, volume);
    }

    // Update the channel cursor now that it probably moved
    channel->buffers_byte_cursor[channel_buffer_to_read_index] = channel_buffer_byte_cursor;

    // Return the actual number of samples copied to output
    return sample_count;
}


// This is used on platforms that need to play individual samples
// themselves (like rp2 port). It returns needed samples at the correct
// rate defined by each type of source
float audio_channel_get_rate_limited_sample(audio_channel_class_obj_t *channel, float volume, bool *complete){
    if(channel->source == NULL){
        return 0.0f;
    }

    if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
        wave_sound_resource_class_obj_t *wave = channel->source;

        // Keep returning current sample until time to get next one
        if(wave->play_counter != 0){
            if(wave->play_counter == wave->play_counter_max){
                wave->play_counter = 0;
            }else{
                wave->play_counter++;
                return wave->last_sample;
            }
        }

        audio_channel_get_samples(channel, &wave->last_sample, 1, volume, complete);
        wave->play_counter++;

        return wave->last_sample;
    }else if(mp_obj_is_type(channel->source, &tone_sound_resource_class_type)){
        // tone_sound_resource_class_obj_t *tone = channel->source;
        float sample = 0.0f;
        audio_channel_get_samples(channel, &sample, 1, volume, complete);
        return sample;
    }else if(mp_obj_is_type(channel->source, &rtttl_sound_resource_class_type)){
        // rtttl_sound_resource_class_obj_t *rtttl = channel->source;
    }

    return 0.0f;
}


/*  --- doc ---
    NAME: stop
    ID: audio_channel_stop
    DESC: Stops audio playing on channel
    RETURN: None
*/ 
mp_obj_t audio_channel_stop(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("AudioChannel: Stopping!");

    audio_channel_class_obj_t *channel = self_in;

    // Set true, busy adjusting source, don't want the ISR doing
    // anything with this channel when in the middle of it
    channel->busy = true;

    // Make sure that if this channel has a source, that
    // the reference from that source to this channel is
    // unlinked. Took care of it here anyway
    if(channel->source != NULL){
        if(mp_obj_is_type(channel->source, &wave_sound_resource_class_type)){
            ((wave_sound_resource_class_obj_t*)channel->source)->channel = NULL;
        }else if(mp_obj_is_type(channel->source, &tone_sound_resource_class_type)){
            ((tone_sound_resource_class_obj_t*)channel->source)->channel = NULL;
        }else if(mp_obj_is_type(channel->source, &rtttl_sound_resource_class_type)){
            ((rtttl_sound_resource_class_obj_t*)channel->source)->channel = NULL;
        }
    }

    // Sets source to NULL and resets other properties
    audio_channel_reset(channel);

    // Set back to false now that we're done readjusting the channel
    channel->busy = false;

    ENGINE_INFO_PRINTF("Done stopping!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(audio_channel_stop_obj, audio_channel_stop);


/*  --- doc ---
    NAME: AudioChannel
    ID: AudioChannel
    DESC: Object for controlling audio on one of the 4 available channels
    ATTR: [type=function]   [name={ref_link:audio_channel_play}]    [value=function]
    ATTR: [type=function]   [name={ref_link:audio_channel_stop}]    [value=function]
    ATTR: [type=object]     [name=source]                           [value={ref_link:WaveSoundResource} or {ref_link:ToneSoundResource}]
    ATTR: [type=float]      [name=gain]                             [value=any (default is 1.0)]
    ATTR: [type=float]      [name=time]                             [value=0.0 to time at the end of the media being played (if there is an end) (read-only and is updated to represent the current time in seconds of teh media being played)]
    ATTR: [type=float]      [name=amplitude]                        [value=-1.0 to 1.0 (the amplitude of the last sample played on this channel, read-only)]
    ATTR: [type=boolean]    [name=loop]                             [value=True or False (whether to loop audio or not)]
    ATTR: [type=boolean]    [name=done]                             [value=True or False (set True when audio finishes playing if not looping, read-only)]
*/ 
static void audio_channel_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing AudioChannel attr");

    audio_channel_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_play:
                destination[0] = MP_OBJ_FROM_PTR(&audio_channel_play_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_stop:
                destination[0] = MP_OBJ_FROM_PTR(&audio_channel_stop_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_source:
                destination[0] = self->source;
            break;
            case MP_QSTR_gain:
                destination[0] = mp_obj_new_float(self->gain);
            break;
            case MP_QSTR_time:
                destination[0] = mp_obj_new_float(self->time);
            break;
            case MP_QSTR_amplitude:
                destination[0] = mp_obj_new_float(self->amplitude);
            break;
            case MP_QSTR_loop:
                destination[0] = mp_obj_new_bool(self->loop);
            break;
            case MP_QSTR_done:
                destination[0] = mp_obj_new_bool(self->done);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_source:
                self->source = destination[1];
            break;
            case MP_QSTR_gain:
                self->gain = engine_math_clamp(mp_obj_get_float(destination[1]), 0.0f, 1.0f);
            break;
            // case MP_QSTR_time:
            //     self->time = mp_obj_get_float(destination[1]);
            // break;
            case MP_QSTR_amplitude:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("AudioChannel: ERROR: Setting the sample amplitude is not allowed!"));
            break;
            case MP_QSTR_loop:
                self->loop = mp_obj_get_int(destination[1]);
            break;
            // case MP_QSTR_done:
            //     self->done = mp_obj_get_int(destination[1]);
            // break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
static const mp_rom_map_elem_t audio_channel_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(audio_channel_class_locals_dict, audio_channel_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    audio_channel_class_type,
    MP_QSTR_AudioChannel,
    MP_TYPE_FLAG_NONE,

    make_new, audio_channel_class_new_dummy,
    attr, audio_channel_class_attr,
    locals_dict, &audio_channel_class_locals_dict
);