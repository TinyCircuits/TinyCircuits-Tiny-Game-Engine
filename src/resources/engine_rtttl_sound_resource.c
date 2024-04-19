#include "engine_rtttl_sound_resource.h"
#include "audio/engine_audio_channel.h"
#include "audio/engine_audio_module.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include "math/engine_math.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "../lib/cglm/include/cglm/util.h"


// Class required functions
STATIC void rtttl_sound_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): RTTTLSoundResource");
}


float ENGINE_FAST_FUNCTION(rtttl_sound_resource_get_sample)(rtttl_sound_resource_class_obj_t *self){
    
    return 0.0f;
}


mp_obj_t rtttl_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New RTTTLSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    rtttl_sound_resource_class_obj_t *self = m_new_obj_with_finaliser(rtttl_sound_resource_class_obj_t);
    self->base.type = &rtttl_sound_resource_class_type;
    self->channel = NULL;

    // Open one-time file
    engine_file_open(mp_obj_str_get_str(args[0]));

    // Close one-time file
    engine_file_close();

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
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
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
    print, rtttl_sound_resource_class_print,
    attr, rtttl_sound_resource_class_attr,
    locals_dict, &rtttl_sound_resource_class_locals_dict
);