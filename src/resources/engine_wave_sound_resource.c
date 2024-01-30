#include "engine_wave_sound_resource.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


// Class required functions
STATIC void wave_sound_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): WaveSoundResource");
}


uint32_t wave_sound_resource_fill_destination(void *self_in, uint8_t *data_buffer_out, uint32_t offset, uint32_t size){
    sound_resource_base_class_obj_t *self = self_in;
    uint32_t size_max = fminf(self->total_data_size-offset, size);
    data_buffer_out = ((uint8_t*)self->extra_data)+offset;
    return size_max;
}


mp_obj_t wave_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New WaveSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    sound_resource_base_class_obj_t *self = m_new_obj_with_finaliser(sound_resource_base_class_obj_t);
    self->base.type = &wave_sound_resource_class_type;
    self->get_data_buffer = &wave_sound_resource_fill_destination;

    // Init mutex used to sync cores between core0 (user Python code)
    // and core1 (audio playback)
    mp_thread_mutex_init(&self->mutex);

    // Wave parsing: https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
    //               https://www.aelius.com/njh/wavemetatools/doc/riffmci.pdf
    //               https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    unsigned char temporary_string[] = {' ', ' ', ' ', ' ', '\0'};
    uint32_t file_size = 0;
    uint16_t format_type = 0;
    uint16_t channel_count = 0;

    engine_file_open(mp_obj_str_get_str(args[0]));

    // Check that this is a riff file
    engine_file_read(temporary_string, 4);
    if(strcmp(temporary_string, "RIFF\0") != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: file does not start with 'RIFF', may not be a wave file"));
    }

    // Get file size
    file_size = engine_file_get_u32(4);

    // Check that this is a wave file
    engine_file_read(temporary_string, 4);
    if(strcmp(temporary_string, "WAVE\0") != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: file type header is not 'WAVE', incorrect file type"));
    }

    // Get and check that the sample format type is PCM
    format_type = engine_file_get_u16(20);
    if(format_type != 1){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: samples are not in PCM format. Compressed formats are not supported"));
    }

    // Get and check that the number of channels is 1 (do not support multi-channel wave files)
    channel_count = engine_file_get_u16(22);
    if(channel_count != 1){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: Either no channels detected or too many. Only single channel wave files are supported"));
    }

    // Get sample rate that this wave file should be played back at and
    // the bytes that each sample needs (gets bits per sample adn divides
    // by size of a byte)
    self->sample_rate = engine_file_get_u32(24);
    self->bytes_per_sample = engine_file_get_u16(34) / 8;

    // Check for data marker
    engine_file_read(temporary_string, 4);
    if(strcmp(temporary_string, "data\0") != 0){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: missing wave 'data' marker, no data?"));
    }

    // Get how large the data section is in bytes
    self->total_data_size = engine_file_get_u32(40);

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
    self->extra_data = (uint8_t*)engine_resource_get_space(self->total_data_size, false);
    engine_resource_start_storing(self->extra_data, false);

    for(uint32_t i=0; i<self->total_data_size; i++){
        engine_resource_store_u8(engine_file_get_u8(44 + i));
    }

    // Stop storing so that any pending non completely filled pages are written
    engine_resource_stop_storing();
    engine_file_close();
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t wave_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("WaveSoundResource: Deleted (freeing sound data)");

    // sound_resource_base_class_obj_t *self = self_in;
    // Since this is stored in contigious flash space and
    // nothing exists to get rid of it, yet, do nothing

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(wave_sound_resource_class_del_obj, wave_sound_resource_class_del);


STATIC void wave_sound_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing WaveSoundResource attr");

    sound_resource_base_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&wave_sound_resource_class_del_obj);
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
STATIC const mp_rom_map_elem_t wave_sound_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(wave_sound_resource_class_locals_dict, wave_sound_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    wave_sound_resource_class_type,
    MP_QSTR_WaveSoundResource,
    MP_TYPE_FLAG_NONE,

    make_new, wave_sound_resource_class_new,
    print, wave_sound_resource_class_print,
    attr, wave_sound_resource_class_attr,
    locals_dict, &wave_sound_resource_class_locals_dict
);