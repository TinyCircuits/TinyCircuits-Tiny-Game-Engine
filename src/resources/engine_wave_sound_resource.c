#include "engine_wave_sound_resource.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include <stdlib.h>
#include <string.h>


// Class required functions
STATIC void wave_sound_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): WaveSoundResource");
}


mp_obj_t wave_sound_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New WaveSoundResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    sound_resource_base_class_obj_t *self = m_new_obj_with_finaliser(sound_resource_base_class_obj_t);
    self->base.type = &wave_sound_resource_class_type;

    // // Wave parsing: https://truelogic.org/wordpress/2015/09/04/parsing-a-wav-file-in-c/
    // //               https://www.aelius.com/njh/wavemetatools/doc/riffmci.pdf
    // //               https://www.mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
    // unsigned char riff_string[5];
    // riff_string[4] = '\0';
    // uint32_t file_size = 0;
    // unsigned char wave_string[5];
    // wave_string[4] = '\0';
    // unsigned char data_marker_string[5];
    // data_marker_string[4] = '\0';
    // uint32_t data_size = 0;
    // uint16_t format_type = 0;

    // engine_file_open(mp_obj_str_get_str(args[0]));

    // // Check that this is a riff file
    // engine_file_read(riff_string, 4);
    // if(strcmp(riff_string, "RIFF\0") != 0){
    //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: file does not start with 'RIFF', may not be a wave file"));
    // }

    // file_size = engine_file_get_u32(4);

    // // Check that this is a wave file
    // engine_file_read(wave_string, 4);
    // if(strcmp(wave_string, "WAVE\0") != 0){
    //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: file type header is not 'WAVE', incorrect file type"));
    // }

    // format_type = engine_file_get_u16(20);
    // if(format_type != 1){
    //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: samples are not in PCM format. Compressed formats are not supported"));
    // }

    // self->channel_count = engine_file_get_u16(22);
    // self->sample_rate = engine_file_get_u32(24);
    // self->bits_per_sample = engine_file_get_u16(34);

    // // Check for data marker
    // engine_file_read(data_marker_string, 4);
    // if(strcmp(data_marker_string, "data\0") != 0){
    //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("WaveSoundResource Error: missing wave 'data' marker, no data?"));
    // }

    // data_size = engine_file_get_u32(40);
    // uint32_t byte_count_per_channel = (data_size / self->channel_count);
    // uint32_t sample_count_per_channel = byte_count_per_channel / (self->bits_per_sample / 8);

    // // Do some information printing:
    // ENGINE_INFO_PRINTF("WaveSoundResource: Wave parameters parsed from '%s':", mp_obj_str_get_str(args[0]));
    // ENGINE_INFO_PRINTF("\tbits_per_sample:\t\t%lu", self->bits_per_sample);
    // ENGINE_INFO_PRINTF("\tfile_size:\t\t\t%lu", file_size);
    // ENGINE_INFO_PRINTF("\tdata_size:\t\t\t%lu", data_size);
    // ENGINE_INFO_PRINTF("\tbyte_count_per_channel:\t\t%lu", byte_count_per_channel);
    // ENGINE_INFO_PRINTF("\tsample_count_per_channel:\t%lu", sample_count_per_channel);
    // ENGINE_INFO_PRINTF("\tformat_type:\t\t\t%lu", format_type);
    // ENGINE_INFO_PRINTF("\tchannel_count:\t\t\t%d", self->channel_count);
    // ENGINE_INFO_PRINTF("\tsample_rate:\t\t\t%lu", self->sample_rate);

    // // Allocate space in ram for audio data, use mp heap.
    // // Allocate space for pointers to otehr locations (the rows)
    // self->channel_data = m_malloc(sizeof(uint8_t*) * self->channel_count);

    // // Allocate the space inside the rows for the actual audio data.
    // // All audio data is stored using 16-bits of space for amplitudes
    // // (may allow for 8-bit in the future: TODO. That would require
    // // scaling samples at the output)
    // for(uint16_t icx=0; icx<self->channel_count; icx++){
    //     self->channel_data[icx] = m_malloc(byte_count_per_channel);
    // }

    // engine_file_close();






    // uint16_t bitmap_id = engine_file_get_u16(0);
    // uint32_t bitmap_pixel_data_offset = engine_file_get_u32(10);
    // uint32_t bitmap_width = engine_file_get_u32(18);
    // uint32_t bitmap_height = engine_file_get_u32(22);
    // uint32_t bitmap_bits_per_pixel = engine_file_get_u16(28);
    // uint32_t bitmap_data_size = engine_file_get_u32(34);

    // ENGINE_INFO_PRINTF("TextureResource: BMP Parameters parsed from '%s':", mp_obj_str_get_str(args[0]));
    // ENGINE_INFO_PRINTF("\tbitmap_id:\t\t\t%d", bitmap_id);
    // ENGINE_INFO_PRINTF("\tbitmap_pixel_data_offset:\t%lu", bitmap_pixel_data_offset);
    // ENGINE_INFO_PRINTF("\tbitmap_width:\t\t\t%lu", bitmap_width);
    // ENGINE_INFO_PRINTF("\tbitmap_height:\t\t\t%lu", bitmap_height);
    // ENGINE_INFO_PRINTF("\tbitmap_bits_per_pixel:\t\t%lu", bitmap_bits_per_pixel);
    // ENGINE_INFO_PRINTF("\tbitmap_data_size:\t\t%lu", bitmap_data_size);

    // // Check header ID field
    // if(bitmap_id != 19778){
    //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP header ID field incorrect!"));
    // }

    // if(bitmap_bits_per_pixel != 16){
    //     mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP has wrong color depth! Expects 16-bits per pixel"));
    // }

    // self->width = bitmap_width;
    // self->height = bitmap_height;
    
    // // Pass the size of the pixel data, and if it is in fast ram or not
    // self->data = (uint16_t*)engine_resource_get_space(bitmap_data_size, self->in_ram);
    // engine_resource_start_storing((uint8_t*)self->data, self->in_ram);

    // uint16_t bitmap_pixel_src_x = 0;
    // uint16_t bitmap_pixel_src_y = bitmap_height-1;
    // for(uint32_t bitmap_pixel_dest_index=0; bitmap_pixel_dest_index<bitmap_width*bitmap_height; bitmap_pixel_dest_index++){
        
    //     uint32_t bitmap_pixel_src_index = bitmap_pixel_src_y * bitmap_width + bitmap_pixel_src_x;
    //     engine_resource_store_u16(engine_file_get_u16(bitmap_pixel_data_offset+bitmap_pixel_src_index*2));

    //     bitmap_pixel_src_x++;
    //     if(bitmap_pixel_src_x >= bitmap_width){
    //         bitmap_pixel_src_x = 0;
    //         bitmap_pixel_src_y--;
    //     }
    // }

    // engine_resource_stop_storing();
    // engine_file_close();
    
    return MP_OBJ_FROM_PTR(self);
}


uint32_t wave_sound_resource_fill_destination(uint8_t *destination, uint32_t offset, uint32_t size){

}


// Class methods
STATIC mp_obj_t wave_sound_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("WaveSoundResource: Deleted (freeing sound data)");

    sound_resource_base_class_obj_t *self = self_in;

    // m_free(self->data);

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