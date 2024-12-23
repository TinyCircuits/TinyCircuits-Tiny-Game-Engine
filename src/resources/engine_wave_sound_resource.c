#include "engine_wave_sound_resource.h"
#include "audio/engine_audio_channel.h"
#include "audio/engine_audio_module.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


uint8_t *wave_sound_resource_fill_destination(void *channel_in, uint32_t max_buffer_size, uint16_t *leftover_size){
    audio_channel_class_obj_t *channel = channel_in;
    sound_resource_base_class_obj_t *source = (sound_resource_base_class_obj_t*)channel->source;

    *leftover_size = (uint16_t)fminf(source->total_data_size - channel->source_byte_offset, max_buffer_size);
    uint8_t *data = ENGINE_BYTEARRAY_OBJ_TO_DATA(source->extra_data);
    return data + channel->source_byte_offset;
}


mp_obj_t wave_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New WaveSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    sound_resource_base_class_obj_t *self = mp_obj_malloc_with_finaliser(sound_resource_base_class_obj_t, &wave_sound_resource_class_type);
    self->base.type = &wave_sound_resource_class_type;
    self->get_data = &wave_sound_resource_fill_destination;
    self->channel = NULL;
    self->play_counter_max = 0;
    self->play_counter = 0;
    self->last_sample = 0.0f;
    self->in_ram = false;

    if(n_args > 1){
        self->in_ram = mp_obj_get_int(args[1]);
    }

    // Wave parsing: https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
    //               https://www.aelius.com/njh/wavemetatools/doc/riffmci.pdf
    //               https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    char temporary_string[] = {' ', ' ', ' ', ' ', '\0'};
    uint32_t file_size = 0;
    uint16_t format_type = 0;
    uint16_t channel_count = 0;

    engine_file_open_read(0, args[0]);

    // Check that this is a riff file
    engine_file_read(0, temporary_string, 4);
    if(strcmp(temporary_string, "RIFF\0") != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: file does not start with 'RIFF', may not be a wave file"));
    }

    // Get file size
    file_size = engine_file_seek_get_u32(0, 4);

    // Check that this is a wave file
    engine_file_read(0, temporary_string, 4);
    if(strcmp(temporary_string, "WAVE\0") != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: file type header is not 'WAVE', incorrect file type"));
    }

    // Get and check that the sample format type is PCM
    format_type = engine_file_seek_get_u16(0, 20);
    if(format_type != 1){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: samples are not in PCM format. Compressed formats are not supported"));
    }

    // Get and check that the number of channels is 1 (do not support multi-channel wave files)
    channel_count = engine_file_seek_get_u16(0, 22);
    if(channel_count != 1){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: Either no channels detected or too many. Only single channel wave files are supported"));
    }

    // Get sample rate that this wave file should be played back at and
    // the bytes that each sample needs (gets bits per sample and divides
    // by size of a byte)
    self->sample_rate = engine_file_seek_get_u32(0, 24);
    self->play_counter_max = (uint8_t)((1.0f/(float)self->sample_rate) / (1.0f/(float)ENGINE_AUDIO_SAMPLE_RATE));

    self->bytes_per_sample = engine_file_seek_get_u16(0, 34) / 8;

    // Check for data marker
    engine_file_read(0, temporary_string, 4);
    if(strcmp(temporary_string, "data\0") != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: missing wave 'data' marker, no data?"));
    }

    // Get how large the data section is in bytes
    self->total_data_size = engine_file_seek_get_u32(0, 40);

    // Given the size of all the data
    self->total_sample_count = self->total_data_size / self->bytes_per_sample;

    // Print some information:
    ENGINE_INFO_PRINTF("WaveSoundResource: Wave parameters parsed from '%s':", mp_obj_str_get_str(args[0]));
    ENGINE_INFO_PRINTF("\tfile_size:\t\t\t%lu", file_size);
    ENGINE_INFO_PRINTF("\ttotal_data_size:\t\t%lu", self->total_data_size);
    ENGINE_INFO_PRINTF("\tformat_type:\t\t\t%lu", format_type);
    ENGINE_INFO_PRINTF("\tchannel_count:\t\t\t%d", channel_count);
    ENGINE_INFO_PRINTF("\tsample_rate:\t\t\t%lu", self->sample_rate);
    ENGINE_INFO_PRINTF("\ttotal_sample_count:\t\t%lu", self->total_sample_count);
    ENGINE_INFO_PRINTF("\tbytes_per_sample:\t\t%lu", self->bytes_per_sample);

    // Get space in continuous flash area (stored in extra data for this type 'wave_sound_resource_class_type')
    self->extra_data = engine_resource_get_space_bytearray(self->total_data_size, self->in_ram);
    engine_resource_start_storing(self->extra_data, self->in_ram);

    uint8_t temp_buffer[512];
    uint32_t remaining_amount_to_read = self->total_data_size;

    while(remaining_amount_to_read != 0){
        uint16_t amount_to_read = MIN(512, remaining_amount_to_read);
        uint16_t read_amount = engine_file_read(0, temp_buffer, amount_to_read);
        remaining_amount_to_read -= read_amount;

        for(uint16_t i=0; i<read_amount; i++){
            engine_resource_store_u8(temp_buffer[i]);
        }
    }

    // Stop storing so that any pending non completely filled pages are written
    engine_resource_stop_storing();
    engine_file_close(0);
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
static mp_obj_t wave_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("WaveSoundResource: Deleted (freeing sound data)");

    sound_resource_base_class_obj_t *self = self_in;
    audio_channel_class_obj_t *channel = self->channel;

    // This is very important! Need to make sure to set channel source this source is
    // related to NULL. Otherwise, even though this source gets collected it will not
    // be set to NULL and the audio ISR will try to access invalid memory!!!
    if(channel != NULL){
        audio_channel_stop(channel);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(wave_sound_resource_class_del_obj, wave_sound_resource_class_del);


/*  --- doc ---
    NAME: WaveSoundResource
    ID: WaveSoundResource
    DESC: Holds audio data from a .wav file. `.wav` files can be 8 or 16-bit PCM and only samples rates equal to or less than 22050Hz. Recommended sample rates are: 22050Hz, 11025Hz, 5512Hz, 2756Hz, and 1378Hz
    PARAM:  [type=string]       [name=filepath]     [value=string]
    PARAM:  [type=boolean]      [name=in_ram]       [value=True or False (default: False)]
    ATTR:   [type=bytearray]    [name=data]         [value=value of bytearray containing the audio samples]
    ATTR:   [type=float]        [name=duration]     [value=length of wave file in seconds (read-only)]
    ATTR:   [type=int]          [name=sample_rate]  [value=rate that samples are played in Hz (read-only)]                                                                                                                                               
*/ 
static void wave_sound_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing WaveSoundResource attr");

    sound_resource_base_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&wave_sound_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_data:
                destination[0] = self->extra_data;
            break;
            case MP_QSTR_duration:
                destination[0] = mp_obj_new_float((float)self->total_sample_count / (float)self->sample_rate);
            break;
            case MP_QSTR_sample_rate:
                destination[0] = mp_obj_new_int(self->sample_rate);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_data:
                self->extra_data = destination[1];
            break;
            case MP_QSTR_duration:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource: ERROR: Setting the duration it now allowed!"));
            break;
            case MP_QSTR_sample_rate:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource: ERROR: Setting the sample rate it now allowed!"));
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
static const mp_rom_map_elem_t wave_sound_resource_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(wave_sound_resource_class_locals_dict, wave_sound_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    wave_sound_resource_class_type,
    MP_QSTR_WaveSoundResource,
    MP_TYPE_FLAG_NONE,

    make_new, wave_sound_resource_class_new,
    attr, wave_sound_resource_class_attr,
    locals_dict, &wave_sound_resource_class_locals_dict
);