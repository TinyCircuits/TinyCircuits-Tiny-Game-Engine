#include "engine_texture_resource.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include <stdlib.h>
#include <math.h>


mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TextureResource");
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    texture_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(texture_resource_class_obj_t, &texture_resource_class_type);
    self->base.type = &texture_resource_class_type;

    // Set flag indicating if file data is to be stored in
    // ram or not (faster if stored in ram, up to programmer)
    if(n_args == 2){
        self->in_ram = mp_obj_get_int(args[1]);
    }else{
        self->in_ram = false;
    }

    // Always loaded into ram on unix port
    #if defined(__unix__)
        self->in_ram = true;
    #endif

    // BMP parsing: https://en.wikipedia.org/wiki/BMP_file_format
    engine_file_open_read(0, args[0]);

    uint16_t bitmap_id = engine_file_seek_get_u16(0, 0);
    uint32_t bitmap_file_size = engine_file_seek_get_u32(0, 2);
    uint32_t bitmap_pixel_data_offset = engine_file_seek_get_u32(0, 10);
    uint32_t bitmap_width = engine_file_seek_get_u32(0, 18);
    uint32_t bitmap_height = engine_file_seek_get_u32(0, 22);
    uint32_t bitmap_bits_per_pixel = engine_file_seek_get_u16(0, 28);
    // uint32_t bitmap_data_size = engine_file_seek_get_u32(0, 34); not all exporters actually export this, need to infer a different way

    if(bitmap_bits_per_pixel != 16){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Expected 16-bit pixel data, got %d-bit"), bitmap_bits_per_pixel);
    }

    // Because some exporters do not use the pixel data size field
    // correctly, calculate it based on the full file size and the
    // offset into the file to the pixel array. Need to do it this
    // way since pixel data rows are padded
    uint32_t bitmap_data_size = bitmap_file_size - bitmap_pixel_data_offset;

    ENGINE_INFO_PRINTF("TextureResource: BMP parameters parsed from '%s':", mp_obj_str_get_str(args[0]));
    ENGINE_INFO_PRINTF("\tbitmap_id:\t\t\t%d", bitmap_id);
    ENGINE_INFO_PRINTF("\bitmap_file_size:\t\t\t%d", bitmap_file_size);
    ENGINE_INFO_PRINTF("\tbitmap_pixel_data_offset:\t%lu", bitmap_pixel_data_offset);
    ENGINE_INFO_PRINTF("\tbitmap_width:\t\t\t%lu", bitmap_width);
    ENGINE_INFO_PRINTF("\tbitmap_height:\t\t\t%lu", bitmap_height);
    ENGINE_INFO_PRINTF("\tbitmap_bits_per_pixel:\t\t%lu", bitmap_bits_per_pixel);
    ENGINE_INFO_PRINTF("\tbitmap_data_size:\t\t%lu", bitmap_data_size);

    // Check header ID field
    if(bitmap_id != 19778){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP header ID field incorrect! Not a BMP file or file doesn't exist!"));
    }

    if(bitmap_bits_per_pixel != 16){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP has wrong color depth! Expects 16-bits per pixel"));
    }

    self->width = bitmap_width;
    self->height = bitmap_height;

    // https://en.wikipedia.org/wiki/BMP_file_format#:~:text=optional%20color%20list.-,Pixel%20storage,-%5Bedit%5D
    // Due to the format and padding to multiples of 4
    // in bitmaps, an arrow drawn pointing up in a 7 x 7
    // bitmap will be stored as (- is padding):
    //
    // X X X . X X X -
    // X X X . X X X -
    // X X X . X X X -
    // X X X . X X X -
    // X . X . X . X -
    // X X . . . X X -
    // X X X . X X X -
    //
    // This means, when using the serial storing API
    // below, we need grab chunks of bitmap data from
    // end to start, and store pixels going as:
    // 1. Start at at the bottom-left of the above diagram adn store that pixel
    // 2. Move to the right and store those pixels
    // 3. After moving to the right and reaching the end of the padding, go up a row and back to the start on the left

    // This is the real width of the bitmap pixel data array, needed for calculating offsets
    uint16_t padded_bitmap_width = (uint16_t)(ceilf((float)bitmap_width / 2.0f) * 2.0f);

    // Get space for texture in SRAM/FLASH and start storing API
    self->data = engine_resource_get_space_bytearray(bitmap_data_size, self->in_ram);
    engine_resource_start_storing(self->data, self->in_ram);

    uint8_t temp_row_pixels_buffer[256];

    // Fetch pixel data from filesystem row by row from end to start
    for(int32_t y=bitmap_height-1; y>=0 ; y--){
        // Seek to start of bitmap pixel row at current y
        // and store how many bytes we'll need to fetch
        // for this row
        uint32_t offset = y * padded_bitmap_width + 0;
        engine_file_seek(0, bitmap_pixel_data_offset + (offset*2) + 0, MP_SEEK_SET);
        uint16_t remaining_width = bitmap_width * 2;

        while(remaining_width != 0){
            // Read up to 256 bytes of pixel data at a time
            uint16_t amount_to_read = MIN(256, remaining_width);

            uint16_t read_amount = engine_file_read(0, temp_row_pixels_buffer, amount_to_read);
            remaining_width -= read_amount;

            for(uint16_t i=0; i<read_amount/2; i++){
                engine_resource_store_u16(((uint16_t*)temp_row_pixels_buffer)[i]);
            }
        }
    }

    engine_resource_stop_storing();
    engine_file_close(0);

    

    // engine_file_seek(0, bitmap_pixel_data_offset, MP_SEEK_SET);

    // // 
    // // Need to know how many pixels of padding there are in each
    // // row so that the pixel data can be extracted correctly
    // // (working with pixels here whereas it is padded in bytes)
    // uint16_t padded_multiple_4_width = 

    // for(uint32_t bitmap_pixel_dest_index=0; bitmap_pixel_dest_index<bitmap_width*bitmap_height; bitmap_pixel_dest_index++){
        
    //     // Each row has padded bytes to make each row a multiple
    //     // of 4, use that width for calculating the index
    //     uint32_t bitmap_pixel_src_index = bitmap_pixel_src_y * padded_multiple_4_width + bitmap_pixel_src_x;
    //     engine_resource_store_u16(engine_file_seek_get_u16(0, bitmap_pixel_data_offset + (bitmap_pixel_src_index*2)));

    //     bitmap_pixel_src_x++;
    //     if(bitmap_pixel_src_x >= bitmap_width){
    //         bitmap_pixel_src_x = 0;
    //         bitmap_pixel_src_y--;
    //     }
    // }

    // engine_resource_stop_storing();
    // engine_file_close(0);
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
static mp_obj_t texture_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("TextureResource: Deleted (freeing texture data)");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(texture_resource_class_del_obj, texture_resource_class_del);


uint16_t texture_resource_get_pixel(texture_resource_class_obj_t *texture, uint32_t offset){
    uint16_t *texture_data = ((mp_obj_array_t*)texture->data)->items;
    return texture_data[offset];
}


/*  --- doc ---
    NAME: TextureResource
    ID: TextureResource
    DESC: Object that holds information about bitmaps. The file needs to be a 16-bit RGB565 .bmp file
    PARAM:  [type=string]       [name=filepath]  [value=string]
    PARAM:  [type=boolean]      [name=in_ram]    [value=True of False (False by default)]
    ATTR:   [type=float]        [name=width]     [value=any (read-only)]
    ATTR:   [type=float]        [name=height]    [value=any (read-only)]
    ATTR:   [type=bytearray]    [name=data]      [value=RGB565 bytearray (note, if in_ram is False, then writing to this is not a valid operation)]
*/ 
static void texture_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing TextureResource attr");

    texture_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&texture_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_width:
                destination[0] = mp_obj_new_int(self->width);
            break;
            case MP_QSTR_height:
                destination[0] = mp_obj_new_int(self->height);
            break;
            case MP_QSTR_data:
                destination[0] = self->data;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_data:
            {
                mp_obj_array_t *new_bytearray = destination[1];
                mp_obj_array_t *cur_bytearray = self->data;
                if(cur_bytearray->len != new_bytearray->len){
                    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Can't set texture data to new bytearray, lengths do not match!"));
                }
                self->data = destination[1];
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
static const mp_rom_map_elem_t texture_resource_class_locals_dict_table[] = {

};
static MP_DEFINE_CONST_DICT(texture_resource_class_locals_dict, texture_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    texture_resource_class_type,
    MP_QSTR_TextureResource,
    MP_TYPE_FLAG_NONE,

    make_new, texture_resource_class_new,
    attr, texture_resource_class_attr,
    locals_dict, &texture_resource_class_locals_dict
);