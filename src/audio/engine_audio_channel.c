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
    self->buffer = (uint8_t*)malloc(CHANNEL_BUFFER_SIZE);
    self->buffer_byte_offset = UINT16_MAX;
    
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