#include "engine_audio_channel.h"
#include "math/engine_math.h"
#include "debug/debug_print.h"
#include <stdlib.h>
#include <string.h>

#include "resources/engine_sound_resource_base.h"
#include "resources/engine_wave_sound_resource.h"
#include "resources/engine_tone_sound_resource.h"


// Class required functions
STATIC void audio_channel_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): AudioChannel");
}


mp_obj_t audio_channel_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New AudioChannel");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // Allocate on C heap so it doesn't get collected. Each channel lives for as long as the device is on.
    // This was verified to work by disabling the audio interrupts and printing out `print((dir(chan)))` of
    // a channel started by `chan = engine_audio.play(...)`
    audio_channel_class_obj_t *self = (audio_channel_class_obj_t*)malloc(sizeof(audio_channel_class_obj_t));
    self->base.type = &audio_channel_class_type;

    self->source = NULL;   // Set to NULL to indicate that source/channel not active
    self->source_byte_offset = 0;
    self->gain = 1.0f;
    self->time = 0.0f;
    self->done = true;
    self->loop = false;
    self->buffers[0] = (uint8_t*)malloc(CHANNEL_BUFFER_SIZE);   // Use C heap. Easier to avoid gc and we have a consistent number of buffers anyways
    self->buffers[1] = (uint8_t*)malloc(CHANNEL_BUFFER_SIZE);   // Use C heap. Easier to avoid gc and we have a consistent number of buffers anyways
    self->buffers_ends[0] = CHANNEL_BUFFER_SIZE;
    self->buffers_ends[1] = CHANNEL_BUFFER_SIZE;
    self->buffers_byte_offsets[0] = 0;
    self->buffers_byte_offsets[1] = 0;
    self->reading_buffer_index = 0;
    self->filling_buffer_index = 0;
    self->busy = false;

    // Make sure to clear the buffers to zero to avoid static sound
    // when audio first starts playing
    memset(self->buffers[0], 0x0000, sizeof(uint8_t) * CHANNEL_BUFFER_SIZE);
    memset(self->buffers[1], 0x0000, sizeof(uint8_t) * CHANNEL_BUFFER_SIZE);

    // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/dma/hello_dma/hello_dma.c#L21-L30
    // https://github.com/raspberrypi/pico-examples/blob/master/flash/xip_stream/flash_xip_stream.c#L58-L70 (see pg. 127 of rp2040 datasheet: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
    // Get a free DMA channel and panic if there isn't one,
    // get a default DMA config, and set the transfer size
    // to 8-bits since smallest audio sample bit-depth is 8
    // (using 16 would mean we'd copy too much data in 8-bit case)
    #if defined(__unix__)

    #elif defined(__arm__)
        self->dma_channel = dma_claim_unused_channel(true);
        self->dma_config = dma_channel_get_default_config(self->dma_channel);
        channel_config_set_transfer_data_size(&self->dma_config, DMA_SIZE_8);
        channel_config_set_read_increment(&self->dma_config, true);
        channel_config_set_write_increment(&self->dma_config, true);
        // channel_config_set_dreq(&self->dma_config, DREQ_XIP_STREAM); // When this is set DMA never finishes (see pg. 127 of rp2040 datasheet: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
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
    NAME: stop
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
            ((sound_resource_base_class_obj_t*)channel->source)->channel = NULL;
        }else{
            ((tone_sound_resource_class_obj_t*)channel->source)->channel = NULL;
        }
    }

    channel->source = NULL;
    channel->source_byte_offset = 0;
    channel->gain = 1.0f;
    channel->time = 0.0f;
    channel->done = true;
    channel->loop = false;
    channel->buffers_ends[0] = CHANNEL_BUFFER_SIZE;
    channel->buffers_ends[1] = CHANNEL_BUFFER_SIZE;
    channel->buffers_byte_offsets[0] = 0;
    channel->buffers_byte_offsets[1] = 0;
    channel->reading_buffer_index = 0;
    channel->filling_buffer_index = 0;

    // Set back to false now that we're done readjusting the channel
    channel->busy = false;

    ENGINE_INFO_PRINTF("Done stopping!");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(audio_channel_stop_obj, audio_channel_stop);


/*  --- doc ---
    NAME: AudioChannel
    DESC: Object for controlling audio on one of the 4 available channels
    ATTR: [type=function]   [name={ref_link:stop}]          [value=function]
    ATTR: [type=object]     [name=source]                   [value={ref_link:WaveSoundResource}]
    ATTR: [type=float]      [name=gain]                     [value=any (default is 1.0)]
    ATTR: [type=float]      [name=time]                     [value=0.0 to end of sound (read-only)]
    ATTR: [type=boolean]    [name=loop]                     [value=True or False (whether to loop audio or not)]
    ATTR: [type=boolean]    [name=done]                     [value=True or False (set True when audio finishes playing if not looping, read-only)]
*/ 
STATIC void audio_channel_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing AudioChannel attr");

    audio_channel_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
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
STATIC const mp_rom_map_elem_t audio_channel_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(audio_channel_class_locals_dict, audio_channel_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    audio_channel_class_type,
    MP_QSTR_AudioChannel,
    MP_TYPE_FLAG_NONE,

    make_new, audio_channel_class_new_dummy,
    print, audio_channel_class_print,
    attr, audio_channel_class_attr,
    locals_dict, &audio_channel_class_locals_dict
);