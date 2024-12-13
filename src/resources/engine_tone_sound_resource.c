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

#if defined(__EMSCRIPTEN__)
    #include "audio/engine_audio_web.h"
#elif defined(__unix__)
    #include "audio/engine_audio_unix.h"
#elif defined(__arm__)
    #include "audio/engine_audio_rp3.h"
#endif


uint32_t tone_fill_dest(tone_sound_resource_class_obj_t *tone, audio_channel_class_obj_t *channel, uint8_t *output, uint32_t byte_count, bool *complete){
    // Tone is never done providing samples
    *complete = false;

    mp_obj_array_t *data = tone->data;
    uint8_t *items = data->items;


    // // Need to fill the output buffer with as much data as possible,
    // // calculate many sample periods can fit into the output, and then
    // // copy the group that many times
    // uint32_t output_sample_group_fit_count = byte_count / data->len;
    // uint32_t copied_byte_count = data->len * output_sample_group_fit_count;
    // uint32_t output_cursor = 0;

    // for(uint32_t igx=0; igx<output_sample_group_fit_count; igx++){
    //     #if defined(__EMSCRIPTEN__)
    //         engine_audio_web_copy(output+output_cursor, items, data->len);
    //     #elif defined(__unix__)
    //         engine_audio_unix_copy(channel, output+output_cursor, items, data->len);
    //     #elif defined(__arm__)
    //         engine_audio_rp3_copy(channel, output+output_cursor, items, data->len);
    //     #endif

    //     output_cursor += data->len;
    // }

    // return copied_byte_count;


    // Figure which is smaller, desired bytes or length of tone buffer
    byte_count = byte_count < data->len ? byte_count : data->len;

    #if defined(__EMSCRIPTEN__)
        engine_audio_web_copy(output, items, byte_count);
    #elif defined(__unix__)
        engine_audio_unix_copy(channel, output, items, byte_count);
    #elif defined(__arm__)
        engine_audio_rp3_copy(channel, output, items, byte_count);
    #endif

    return byte_count;




    // // Tone is never done providing samples
    // *complete = false;

    // // Tone always places f32 samples into the channel buffer,
    // // based on how many bytes can fit into the channel, calculate
    // // how many sampels to generate
    // uint32_t sample_count = byte_count / 4; // 4 bytes per f32 sample

    // // Keep track of where we are putting bytes in the output
    // uint32_t output_byte_cursor = 0;

    // for(uint32_t isx=0; isx<sample_count; isx++){
    //     float sample = engine_math_fast_sin(tone->omega*tone->time);

    //     memcpy(output+output_byte_cursor, &sample, 4);
    //     output_byte_cursor += 4;

    //     // Sample the sin wave at the same rate (maybe the channel should keep track of time? And pass that here? TODO)
    //     tone->time += ENGINE_AUDIO_SAMPLE_RATE_PERIOD;
    // }

    // return sample_count * 4;
}


uint32_t tone_convert(tone_sound_resource_class_obj_t *tone, uint8_t *channel_buffer, float *output, uint32_t sample_count, float volume){
    for(uint32_t isx=0; isx<sample_count; isx++){
        float sample = (float)channel_buffer[isx];
        sample = sample / (float)UINT8_MAX;
        sample = sample - 0.5f;
        sample = sample * 2.0f;
        output[isx] = sample * volume;
    }

    return sample_count;


    // for(uint32_t isx=0; isx<sample_count; isx++){
    //     float sample = 0.0f;
    //     memcpy(&sample, channel_buffer+isx, 4);
    //     output[isx] = sample * volume;
    // }

    // return sample_count * 4;
}


void tone_sound_resource_set_frequency(tone_sound_resource_class_obj_t *self, float frequency){
    if(frequency <= ENGINE_TONE_MINIMUM_HZ || frequency >= ENGINE_AUDIO_SAMPLE_RATE/2.0f){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("ToneSoundResource: Error: the tone generator can only play frequncies between `%0.3f` and `%0.3f` but got `%.03f`..."), (double)ENGINE_TONE_MINIMUM_HZ, (double)(ENGINE_AUDIO_SAMPLE_RATE/2.0f), (double)frequency);
    }

    // https://www.mathworks.com/matlabcentral/answers/36428-sine-wave-plot#answer_45572
    self->frequency = frequency;

    // For one period (s) how many samples can we take at `ENGINE_AUDIO_SAMPLE_RATE`
    float period = 1.0f / frequency;
    uint32_t sample_count = (uint32_t)(period / (ENGINE_AUDIO_SAMPLE_RATE_PERIOD));

    // Stuff as much tone data into the tone buffer as possible
    uint32_t iterations = ENGINE_TONE_BUFFER_LEN / sample_count;
    // sample_count = ((uint32_t)ENGINE_TONE_BUFFER_LEN / sample_count) * sample_count;

    // ENGINE_PRINTF("%d\n", sample_count);

    // For generating the samples on the sin wave
    float omega = 2.0f * PI * frequency;

    // // For generating the samples on the sin wave
    // self->omega = 2.0f * PI * frequency;
    // self->time = 0.0f;

    // For storing the samples
    mp_obj_array_t *data = self->data;
    uint8_t *items = data->items;

    // Generate the samples
    uint32_t index = 0;
    for(uint32_t itx=0; itx<iterations; itx++){
        float time = 0.0f;

        for(uint32_t isx=0; isx<sample_count; isx++){
            float sample = engine_math_fast_sin(omega * time);
            sample = 0.5f + (0.5f*sample);                        // -1.0 ~ 1.0  ->  0.0 ~ 1.0
            items[index] = (uint8_t)(sample * (float)UINT8_MAX);  //  0.0 ~ 1.0  ->  0 ~ 255
            time += ENGINE_AUDIO_SAMPLE_RATE_PERIOD;
            index++;
        }
    }

    data->len = index;   // 8-bit samples
}


mp_obj_t tone_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New ToneSoundResource");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    tone_sound_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(tone_sound_resource_class_obj_t, &tone_sound_resource_class_type);
    self->base.type = &tone_sound_resource_class_type;
    self->channel = NULL;

    // Allocate and assign space in RAM for tone data
    mp_obj_array_t *array = m_new_obj(mp_obj_array_t);
    array->base.type = &mp_type_bytearray;
    array->typecode = BYTEARRAY_TYPECODE;
    array->free = 0;
    array->len = (size_t)ceilf(ENGINE_TONE_BUFFER_LEN);
    array->items = m_new(byte, array->len);
    self->data = array;

    tone_sound_resource_set_frequency(self, 1000.0f);

    return MP_OBJ_FROM_PTR(self);
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
            case MP_QSTR_data:
                destination[0] = self->data;
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