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
#include "../lib/cglm/include/cglm/ease.h"

enum fade_types {FADE_NONE=0, FADE_DOWN=1, FADE_UP=2};

#define STEP 0.01f

float ENGINE_FAST_FUNCTION(tone_sound_resource_get_sample)(tone_sound_resource_class_obj_t *self){
    float gain = 1.0f;

    // When the frequency of this resource is changed,
    // fade gain to zero, switch f, and then back to 1.0
    if(self->fade_type == FADE_DOWN){
        self->fade_factor += STEP;

        if(self->fade_factor < 1.0f){
            gain = glm_lerp(1.0f, 0.0, self->fade_factor);
        }else{
            gain = 0.0f;
            self->frequency = self->next_frequency;
            self->omega = 2.0f * PI * self->frequency;

            self->fade_type = FADE_UP;
            self->fade_factor = 0.0f;
        }
    }else if(self->fade_type == FADE_UP){
        self->fade_factor += STEP;

        if(self->fade_factor < 1.0f){
            gain = glm_lerp(0.0f, 1.0, self->fade_factor);
        }else{
            self->fade_type = FADE_NONE;
            gain = 1.0f;
        }
    }

    float sample = sinf(self->omega * self->time) * gain;
    self->time += ENGINE_AUDIO_SAMPLE_DT;
    return sample;
}


// Provided a output buffer, starts copy to buffer using platforms's copy
void tone_fill_dest(float *output_sample_buffer, uint32_t start_offset, uint32_t len){
    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)

    #elif defined(__arm__)

    #endif
}


mp_obj_t tone_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New ToneSoundResource");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    tone_sound_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(tone_sound_resource_class_obj_t, &tone_sound_resource_class_type);
    self->base.type = &tone_sound_resource_class_type;
    self->channel = NULL;

    // https://www.mathworks.com/matlabcentral/answers/36428-sine-wave-plot#answer_45572
    self->frequency = 1000.0f;
    self->omega = 2.0f * PI * self->frequency;
    self->time = 0.0f;

    self->next_frequency = 0.0f;
    self->fade_type = FADE_NONE; 
    self->fade_factor = 0.0f;

    return MP_OBJ_FROM_PTR(self);
}


void tone_sound_resource_set_frequency(tone_sound_resource_class_obj_t *self, float frequency){

    if(frequency <= 50.0f || frequency >= ENGINE_AUDIO_SAMPLE_RATE/2.0f){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("ToneSoundResource: Error: the tone generator can only play frequncies between 50Hz and %0.3f. A frequncey of %.03f was being set..."), (double)(ENGINE_AUDIO_SAMPLE_RATE/2.0f), (double)frequency);
    }

    self->next_frequency = frequency;
    self->fade_type = FADE_DOWN;
    self->fade_factor = 0.0f;
}


// Class methods
static mp_obj_t tone_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("ToneSoundResource: Deleted (freeing sound data)");

    tone_sound_resource_class_obj_t *self = self_in;
    audio_channel_class_obj_t *channel = self->channel;

    // This is very important! Need to make sure to set channel source this source is
    // related to NULL. Otherwise, even though this source gets collected it will not
    // be set to NULL and the audio ISR will try to access invalid memory!!!
    if(channel != NULL){
        audio_channel_stop(channel);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(tone_sound_resource_class_del_obj, tone_sound_resource_class_del);


/*  --- doc ---
    NAME: ToneSoundResource
    ID: ToneSoundResource
    DESC: Can be used to play a tone on an audio channel
    ATTR:   [type=float]    [name=frequency]    [value=any]                                                                                                                                                                  
*/ 
static void tone_sound_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing ToneSoundResource attr");

    tone_sound_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&tone_sound_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_frequency:
                destination[0] = mp_obj_new_float(self->frequency);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_frequency:
            {
                tone_sound_resource_set_frequency(self, mp_obj_get_float(destination[1]));
            }
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
static const mp_rom_map_elem_t tone_sound_resource_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(tone_sound_resource_class_locals_dict, tone_sound_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    tone_sound_resource_class_type,
    MP_QSTR_ToneSoundResource,
    MP_TYPE_FLAG_NONE,

    make_new, tone_sound_resource_class_new,
    attr, tone_sound_resource_class_attr,
    locals_dict, &tone_sound_resource_class_locals_dict
);