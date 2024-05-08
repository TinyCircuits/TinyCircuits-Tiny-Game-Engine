#include "engine_rtttl_sound_resource.h"
#include "engine_tone_sound_resource.h"
#include "audio/engine_audio_channel.h"
#include "audio/engine_audio_module.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include "math/engine_math.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "../lib/cglm/include/cglm/util.h"


float ENGINE_FAST_FUNCTION(rtttl_sound_resource_get_sample)(rtttl_sound_resource_class_obj_t *self){
    self->seconds_since_beat += ENGINE_AUDIO_SAMPLE_DT;

    // Only play the next note at rate of `b` or `beats-per-minute`
    if(self->seconds_since_beat >= self->seconds_per_beat){
        self->seconds_since_beat = 0.0f;

        // Get the character at current cursor position
        uint8_t current_char = self->data[self->cursor];


    }

    // Always return the current note that should be playing
    tone_sound_resource_get_sample(self->tone);
    
    

    // // While not a comma, collect note and configure tone afterwards
    // while(current_char != 44 && self->cursor < self->data_size){
    //     self->cursor++;
    //     current_char = self->data[self->cursor];
    // }

    // // Skip comma
    // self->cursor++;

    // if(self->cursor >= self->data_size){
    //     self->cursor = 0;
    // }

    return 0.0f;
}


void rtttl_sound_resource_set_b(rtttl_sound_resource_class_obj_t *self, uint16_t b){
    self->b = b;
    self->seconds_per_beat = 60.0f / (float)self->b;
    self->seconds_since_beat = 0.0f;
}


mp_obj_t rtttl_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New RTTTLSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    rtttl_sound_resource_class_obj_t *self = m_new_obj_with_finaliser(rtttl_sound_resource_class_obj_t);
    self->base.type = &rtttl_sound_resource_class_type;
    self->channel = NULL;
    self->tone = tone_sound_resource_class_new(&tone_sound_resource_class_type, 0, 0, NULL);
    self->data = NULL;
    self->data_size = 0;
    self->cursor = 0;

    // https://www.mobilefish.com/tutorials/rtttl/rtttl_quickguide_specification.html#:~:text=defaults%20are%20assumed%3A-,d%3D4%2Co%3D6%2Cb%3D63.,-The%20data%20section
    self->default_d = 4;
    self->default_o = 6;
    rtttl_sound_resource_set_b(self, 63);

    // Open one-time file
    engine_file_open(mp_obj_str_get_str(args[0]));
    self->data_size = engine_file_size();

    // ### STEP 1: Don't care about the name, subtract name colon from the size ###
    // Need to subtract non-data portion of file size.
    // Subtract a byte until we find first colon or at
    // least stop if no colon is found and the data is all
    // the way subtracted
    uint8_t current_char = engine_file_get_u8(self->cursor);
    while(current_char != 58 && self->data_size != 0){
        self->cursor++;
        self->data_size--;
        current_char = engine_file_get_u8(self->cursor);
    }

    // Skip the colon and subtract the first colon from the size
    self->cursor++;
    self->data_size--;

    // ### STEP 2: Subtract the default value section from the size and get the default values ###
    current_char = engine_file_get_u8(self->cursor);
    while(current_char != 58 && self->data_size != 0){
        // switch(current_char){
        //     case 100:   // d
        //     {
        //         self->default_d = current_char;
        //     }
        //     break;
        //     case 111:   // o
        //     {
        //         self->default_o = current_char;
        //     }
        //     break;
        //     case 98:    // b
        //     {
        //         rtttl_sound_resource_set_b(self, current_char);
        //     }
        //     break;
        // }

        self->cursor++;
        self->data_size--;
        current_char = engine_file_get_u8(self->cursor);
    }

    // Skip the colon and subtract the second colon from the size
    self->cursor++;
    self->data_size--;

    // Get space in ram for notes to live
    self->data = engine_resource_get_space(self->data_size, true);
    engine_resource_start_storing(self->data, true);

    // One byte at a time, copy data from LFS to scratch space
    for(uint32_t idx=0; idx<self->data_size; idx++){
        engine_resource_store_u8(engine_file_get_u8(self->cursor));
        self->cursor++;
    }
    
    // Stop storing and close one-time file
    engine_resource_stop_storing();
    engine_file_close();

    // Reset cursor to play music instead of seek through file
    self->cursor = 0;

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t rtttl_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("RTTTLSoundResource: Deleted (freeing sound data)");

    rtttl_sound_resource_class_obj_t *self = self_in;
    audio_channel_class_obj_t *channel = self->channel;

    // This is very important! Need to make sure to set channel source this source is
    // related to NULL. Otherwise, even though this source gets collected it will not
    // be set to NULL and the audio ISR will try to access invalid memory!!!
    if(channel != NULL){
        audio_channel_stop(channel);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rtttl_sound_resource_class_del_obj, rtttl_sound_resource_class_del);


/*  --- doc ---
    NAME: RTTTLSoundResource
    ID: RTTTLSoundResource
    DESC: Can be used to play a music based on a couple of tones
    ATTR:   [type=float]    [name=frequency]    [value=any]                                                                                                                                                                  
*/ 
STATIC void rtttl_sound_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing RTTTLSoundResource attr");

    rtttl_sound_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&rtttl_sound_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_tone:
                destination[0] = self->tone;
            break;
            case MP_QSTR_tempo:
                destination[0] = mp_obj_new_int(self->b);   // beats per minute
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_tempo:
                self->b = mp_obj_get_int(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rtttl_sound_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(rtttl_sound_resource_class_locals_dict, rtttl_sound_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    rtttl_sound_resource_class_type,
    MP_QSTR_RTTTLSoundResource,
    MP_TYPE_FLAG_NONE,

    make_new, rtttl_sound_resource_class_new,
    attr, rtttl_sound_resource_class_attr,
    locals_dict, &rtttl_sound_resource_class_locals_dict
);