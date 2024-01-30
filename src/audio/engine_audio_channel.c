#include "engine_audio_channel.h"
#include "debug/debug_print.h"
#include <stdlib.h>


// Class required functions
STATIC void audio_channel_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): AudioChannel");
}


mp_obj_t audio_channel_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New AudioChannel");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    audio_channel_class_obj_t *self = m_new_obj_with_finaliser(audio_channel_class_obj_t);
    self->base.type = &audio_channel_class_type;

    self->source = NULL;   // Set to NULL to indicate that source/channel not active
    self->source_byte_offset = 0;
    self->gain = 1.0f;
    self->time = 0.0f;
    self->looping = false;
    self->done = true;
    self->buffer = (uint8_t*)malloc(CHANNEL_BUFFER_SIZE);   // Use C heap. Easier to avoid gc and we have a consistent number of buffers anyways
    self->buffer_end = CHANNEL_BUFFER_SIZE;
    self->buffer_byte_offset = 0;

    // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/dma/hello_dma/hello_dma.c#L21-L30
    // https://github.com/raspberrypi/pico-examples/blob/master/flash/xip_stream/flash_xip_stream.c#L58-L70 (see pg. 127 of rp2040 datasheet)
    // Get a free DMA channel and panic if there isn't one,
    // get a default DMA config, and set the transfer size
    // to 8-bits since smallest audio sample bit-depth is 8
    // (using 16 would mean we'd copy too much data in 8-bit case)
    self->dma_channel = dma_claim_unused_channel(true);
    self->dma_config = dma_channel_get_default_config(self->dma_channel);
    channel_config_set_transfer_data_size(&self->dma_config, DMA_SIZE_8);

    // Do not want to automaticaly increment the read source
    // location and write destination, will keep track of that
    // and manually set it ourselves
    channel_config_set_read_increment(&self->dma_config, false);
    channel_config_set_write_increment(&self->dma_config, false);
    channel_config_set_dreq(&self->dma_config, DREQ_XIP_STREAM);

    // Init mutex used to sync cores between core0 (user Python code)
    // and core1 (audio playback)
    mp_thread_mutex_init(&self->mutex);
    
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


STATIC mp_obj_t audio_channel_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("AudioChannel: Deleted");

    audio_channel_class_obj_t *self = self_in;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(audio_channel_class_del_obj, audio_channel_class_del);


STATIC void audio_channel_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing AudioChannel attr");

    audio_channel_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    // Only user code on core0 should invoke this function (don't see a reason
    // why during playback we would need to use something like mp_load_attr/
    // mp_store_attr). Since that's the case, if we want to load a value
    // on core0, core1 could be writing to mostly any of the channel
    // attributes, so safely wait for core1 to stop using this channel's mutex.
    // If we want to store a value in the channel from core0, core1 could be
    // reading or writing to most any attributes, also wait safely to lock
    // so that core1 will have to wait too.
    mp_thread_mutex_lock(&self->mutex, true);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&audio_channel_class_del_obj);
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
            case MP_QSTR_looping:
                destination[0] = mp_obj_new_bool(self->looping);
            break;
            case MP_QSTR_done:
                destination[0] = mp_obj_new_bool(self->done);
            break;
            default:
                mp_thread_mutex_unlock(&self->mutex);    // Unlock before returning
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_source:
                self->source = destination[1];
            break;
            case MP_QSTR_gain:
                self->gain = mp_obj_get_float(destination[1]);
            break;
            // case MP_QSTR_time:
            //     self->time = mp_obj_get_float(destination[1]);
            // break;
            case MP_QSTR_looping:
                self->looping = mp_obj_get_int(destination[1]);
            break;
            // case MP_QSTR_done:
            //     self->done = mp_obj_get_int(destination[1]);
            // break;
            default:
                mp_thread_mutex_unlock(&self->mutex);    // Unlock before returning
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }

    mp_thread_mutex_unlock(&self->mutex);
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