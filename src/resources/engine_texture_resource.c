#include "engine_texture_resource.h"
#include "debug/debug_print.h"
#include "draw/engine_color.h"
#include "resources/engine_resource_manager.h"
#include "draw/engine_color.h"
#include "math/engine_math.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Size of the buffer used to store large reads from LittleFS
#define TEMP_ROW_BUFFER_SIZE 512

enum bitmap_compression{
	BI_RGB,
	BI_RLE8,
	BI_RLE4,
	BI_BITFIELDS,
	BI_JPEG,
	BI_PNG,
	BI_ALPHABITFIELDS,
	BI_CMYK,
	BI_CMYKRLE8,
	BI_CMYKRLE4,
};


// Make sure all structs are minimally packed
// so that file reads can go directly into these
// https://stackoverflow.com/a/7351714
#pragma pack(push)
#pragma pack(1)

typedef struct bmfh_t{                          // Bitmap header
    uint16_t bf_type;
    uint32_t bf_size;                           // Size of entire file
    uint16_t bf_reserved_1;
    uint16_t bf_reserved_2;
    uint32_t bf_off_bits;                       // Offset from 0 to start of pixel/index data
}bmfh_t;


typedef struct bmih_v1_t{                  
    uint32_t bi_size;                           // Size of this information section
	int32_t bi_width;                           // Width of bitmap in pixels
	int32_t bi_height;                          // Height of bitmap in pixels
	uint16_t bi_planes;
	uint16_t bi_bit_count;                      // Bit-depth of bitmap (1, 2, 4, 8, 16, 24, 32)
	uint32_t bi_compression;                    // Type of compression bitmap uses
	uint32_t bi_size_image;                     // Size of the bitmap image section but no always filled out by exporters
	int32_t bi_x_pels_per_meter;
	int32_t bi_y_pels_per_meter;
	uint32_t bi_clr_used;                       // Number of colors used if indexed image
	uint32_t bi_clr_important;
}bmih_v1_t;


typedef struct bmih_v2_t{                  
    uint32_t bi_red_mask;                       // Mask bits for red channel in pixel data (only useful for >= 16bpp formats)
	uint32_t bi_green_mask;                     // Mask bits for green channel in pixel data (only useful for >= 16bpp formats)
	uint32_t bi_blue_mask;                      // Mask bits for blue channel in pixel data (only useful for >= 16bpp formats)
}bmih_v2_t;


typedef struct bmih_v3_t{                  
    uint32_t bi_alpha_mask;                     // Mask bits for alpha channel in pixel data (only useful for >= 16bpp formats)
}bmih_v3_t;

#pragma pack(pop)


void chunked_read_and_store_row(uint32_t bytes_to_read_and_store){
    // To be able to read from LittleFS fast, create a
    // buffer to store reads up to `TEMP_ROW_BUFFER_SIZE` bytes
    uint8_t temp_row_buffer[TEMP_ROW_BUFFER_SIZE];

    // Read and store the data in chunks
    while(bytes_to_read_and_store != 0){
        // Read up to 512 bytes of pixel data at a time (chunk)
        uint16_t amount_to_read = MIN(TEMP_ROW_BUFFER_SIZE, bytes_to_read_and_store);

        uint16_t read_amount = engine_file_read(0, temp_row_buffer, amount_to_read);
        bytes_to_read_and_store -= read_amount;

        for(uint16_t i=0; i<read_amount; i++){
            engine_resource_store_u8(temp_row_buffer[i]);
        }
    }
}


uint8_t bitmap_get_header_and_info(bmfh_t *header, bmih_v1_t *info_v1, bmih_v2_t *info_v2, bmih_v3_t *info_v3){
    // Start assuming we have a BMP with header version 1
    uint8_t version = 1;
    
    // Read header: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
    engine_file_read(0, header, 14);

    // Read info up to version we care about depending on amount of
    // data in the information section
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
    uint32_t read_info_len = 0;
    read_info_len += engine_file_read(0, info_v1, sizeof(bmih_v1_t));

    if(info_v1->bi_size > read_info_len){
        read_info_len += engine_file_read(0, info_v2, sizeof(bmih_v2_t));
        version = 2;
    }

    if(info_v1->bi_size > read_info_len){
        read_info_len += engine_file_read(0, info_v3, sizeof(bmih_v3_t));
        version = 3;
    }

    // Check that this is a bitmap, is a supported bit-depth, and that it is not compressed
    if(header->bf_type != 19778){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP header ID field incorrect! Not a BMP file or file doesn't exist!"));
    }

    if(info_v1->bi_bit_count > 16){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Only bit-depths of 16 and lower are supported! Got `%d`"), info_v1->bi_bit_count);
    }

    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader#:~:text=For%20uncompressed%20RGB%20formats%2C%20the%20following%20values%20are%20possible
    if(info_v1->bi_compression != BI_BITFIELDS && info_v1->bi_compression != BI_RGB){
        mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: This bitmap uses some kind of compression, only uncompressed bitmaps are supported!"));
    }

    return version;
}


void bitmap_get_and_fill_color_table(uint16_t *color_table, uint16_t color_count){
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-rgbquad
    // Color table in file is in bgr format (not rgb)
    uint8_t bgr[4];

    ENGINE_INFO_PRINTF("\t colors: \t\t\t");
    for(uint16_t color_index=0; color_index<color_count; color_index++){
        // Read the 4 bytes (last is reserved) from
        // color table and convert to 16 bit RGB565
        engine_file_read(0, bgr, 4);
        uint16_t rgb565 = engine_color_16_from_24_bit_rgb(bgr[2], bgr[1], bgr[0]);

        // Color table is always in RAM, write directly to it
        color_table[color_index] = rgb565;

        // Print the original color and converted RGB565
        ENGINE_INFO_PRINTF("%d,%d,%d->%d,%d,%d  ", bgr[2], bgr[1], bgr[0], (rgb565 >> 11) & 0b00011111, (rgb565 >> 5) & 0b00111111, (rgb565 >> 0) & 0b00011111);
    }
    ENGINE_INFO_PRINTF("\n");
}


void get_bit_depth_strides(uint16_t bit_depth, uint16_t pixel_width, uint32_t *unpadded_byte_stride, uint16_t *pixel_stride){
    // https://en.wikipedia.org/wiki/BMP_file_format#:~:text=Each%20row%20in%20the%20Pixel%20array%20is%20padded%20to%20a%20multiple%20of%204%20bytes%20in%20size
    if(bit_depth == 1){
        // Each bit in horizontal byte is represents index into color table 
        // with left-most bits representing pixels at the left of the image
        //  width=16 -> unpadded=ceil(16/8)=2 -> padded = ceil(2/4)*4 = 4 bytes
        //  width=17 -> unpadded=ceil(17/8)=3 -> padded = ceil(3/4)*4 = 4 bytes
        *unpadded_byte_stride = (uint32_t)ceilf((float)pixel_width / 8.0f);
        *pixel_stride = *unpadded_byte_stride*8;
    }else if(bit_depth == 4){
        // Every 4-bit chunk of a each horizontal byte represents index into
        // color table
        //  width=16 -> unpadded=ceil(16/2)=8 -> padded=ceil(8/4)*4 = 8 bytes
        //  width=17 -> unpadded=ceil(17/2)=9 -> padded=ceil(9/4)*4 = 12 bytes
        *unpadded_byte_stride = (uint32_t)ceilf((float)pixel_width / 2.0f);
        *pixel_stride = *unpadded_byte_stride*2;
    }else if(bit_depth == 8){
        // Every 8-bit horizontal byte represents index into color table
        *unpadded_byte_stride = pixel_width;
        *pixel_stride = pixel_width;
    }else{
        // Every group of two 8-bit horizontal bytes represents a 16-bit color
        //  width=16 -> unpadded=16*2=32 -> padded=ceil(32/4)*4 = 32 bytes
        //  width=17 -> unpadded=17*2=34 -> padded=ceil(34/4)*4 = 36 bytes
        *unpadded_byte_stride = pixel_width * 2;
        *pixel_stride = pixel_width;
    }
}


uint16_t texture_resource_get_indexed_pixel(texture_resource_class_obj_t *texture, uint32_t pixel_offset, float *out_alpha){
    mp_obj_array_t *data = texture->data;
    mp_obj_array_t *colors = texture->colors;

    // No matter the bit-depth, calculate the index of the byte
    // containing the bits related to the pixel we're after:
    //
    //               width: 24 -> offset = 18       width: 6 -> offset = 4         width: 3 -> offset = 2
    //                                  v                            vvvv                           vvvv vvvv
    //  Examples: 0000_0000 0000_0000 0010_0000  0000_0000 0000_0000 1111_0000  0000_0000 0000_0000 1111_1111 (the byte index we want to calculate is 2 no matter the bit-depth)
    //  1bit_byte_index = floor((bits_per_pixel * offset) / 8.0f) = floor((1*18)/8) = 2
    //  4bit_byte_index = floor((bits_per_pixel * offset) / 8.0f) = floor((4*4)/8)  = 2
    //  8bit_byte_index = floor((bits_per_pixel * offset) / 8.0f) = floor((8*2)/8)  = 2
    // uint32_t byte_containing_pixel_index = texture->bit_depth*pixel_offset/8;
    uint32_t byte_containing_pixel_index = texture->bit_depth*pixel_offset/8;
    uint8_t byte_containing_pixel = ((uint8_t*)data->items)[byte_containing_pixel_index];

    // Now that we have the byte containing the index information
    // into the color table, extract just those bits to get the index.
    // This only matters for the 1 and 4-bit cases since the 8-bit case
    // already contains the entire bit range that makes up the index:
    //
    //           offset=18  offset=4   offset=2
    //        
    // Examples: 0010_0000  1111_0000  1111_1111
    //
    // 1bit_right_shift_count = (8-bit_depth) - (pixel_offset % (8/bit_depth)) = (8-1) - (18 % (8/1)) = 7 - (18 % 8) = 5
    // 4bit_right_shift_count = (8-bit_depth) - (pixel_offset % (8/bit_depth)) = (8-4) - (4  % (8/4)) = 4 - (4  % 2) = 4
    // 8bit_right_shift_count = (8-bit_depth) - (pixel_offset % (8/bit_depth)) = (8-8) - (2  % (8/8)) = 0 - (2  % 1) = 0
    uint8_t right_shift_count = (8-texture->bit_depth) - (pixel_offset % (8/texture->bit_depth));
    right_shift_count = (right_shift_count/texture->bit_depth)*texture->bit_depth;
    byte_containing_pixel = byte_containing_pixel >> right_shift_count;

    // The bits related to the index into the color table have been shifted all the
    // way to the right, now we need a mask generated from the bit-depth to mask it out:
    //
    //  Examples: 0000_0001  0000_1111  1111_1111
    //
    // 1bit_mask = pow(2, bit_depth)-1 = 2^1 - 1 = 2-1   = 1   = 0b0000_0001
    // 2bit_mask = pow(2, bit_depth)-1 = 2^4 - 1 = 16-1  = 15  = 0b0000_1111
    // 8bit_mask = pow(2, bit_depth)-1 = 2^8 - 1 = 256-1 = 255 = 0b1111_1111
    // https://stackoverflow.com/a/5345369 (raise integer two to a power using bit shifts)
    uint8_t index_into_colors_mask = (1 << texture->bit_depth) - 1;
    uint8_t index_into_colors = byte_containing_pixel & index_into_colors_mask;

    // Get the color from the color table
    return ((uint16_t*)colors->items)[index_into_colors];
}


uint16_t texture_resource_get_16bit_rgb565(texture_resource_class_obj_t *texture, uint32_t pixel_offset, float *out_alpha){
    mp_obj_array_t *data = texture->data;
    return ((uint16_t*)data->items)[pixel_offset];
}


uint16_t texture_resource_get_16bit_axrgb(texture_resource_class_obj_t *texture, uint32_t pixel_offset, float *out_alpha){
    mp_obj_array_t *data = texture->data;

    // Get the 16-bit color that is masked a certain way
    uint16_t pixel = ((uint16_t*)data->items)[pixel_offset];

    // Mask out the values
    //                           A RRRRR GGGGG BBBBB
    //  Example:  pixel_1555 = 0b1_11011_00100_10101
    //            a_mask     = 0b1_00000_00000_00000 -> a = pixel_1555 & a_mask = 0b1_11011_00100_10101 & 0b1_00000_00000_00000 = 0b1_00000_00000_00000
    //            r_mask     = 0b0_11111_00000_00000 -> r = pixel_1555 & r_mask = 0b1_11011_00100_10101 & 0b0_11111_00000_00000 = 0b0_11011_00000_00000
    //            g_mask     = 0b0_00000_11111_00000 -> g = pixel_1555 & g_mask = 0b1_11011_00100_10101 & 0b0_00000_11111_00000 = 0b0_00000_00100_00000
    //            b_mask     = 0b0_00000_00000_11111 -> b = pixel_1555 & g_mask = 0b1_11011_00100_10101 & 0b0_00000_00000_11111 = 0b0_00000_00000_10101
    uint16_t a = pixel & texture->alpha_mask;
    uint16_t r = pixel & texture->red_mask;
    uint16_t g = pixel & texture->green_mask;
    uint16_t b = pixel & texture->blue_mask;

    // The shift amounts are calculated once at the end of
    // creating the texture resource
    a = a >> texture->a_mask_right_shift_amount;
    r = r >> texture->r_mask_right_shift_amount;
    g = g >> texture->g_mask_right_shift_amount;
    b = b << texture->b_mask_left_shift_amount;

    // Alpha is special and is output as 0.0 ~ 1.0
    if(out_alpha != NULL) *out_alpha = engine_math_map((float)a, 0.0f, (float)(texture->alpha_mask >> texture->a_mask_right_shift_amount), 0.0f, 1.0f);

    pixel = 0;
    pixel |= (r << 11);
    pixel |= (g << 5);
    pixel |= (b << 0);

    return pixel;
}


void create_blank_from_params(texture_resource_class_obj_t *self, mp_obj_t width, mp_obj_t height, mp_obj_t color, mp_obj_t dit_depth){
    uint16_t blank_width = mp_obj_get_int(width);
    uint16_t blank_height = mp_obj_get_int(height);
    uint32_t blank_pixel_count = blank_width * blank_height;
    uint16_t blank_color = 0xffff;
    uint16_t blank_bit_depth = 16;
    uint32_t blank_data_bytes_size = 0;

    // Figure out the color to fill it with
    if(color != mp_const_none){
        // ALready know it's one of these, figure out which
        if(mp_obj_is_type(color, &const_color_class_type) || mp_obj_is_type(color, &color_class_type)){
            blank_color = ((color_class_obj_t*)color)->value;
        }else{
            blank_color = mp_obj_get_int(color);
        }
    }

    if(dit_depth != mp_const_none){
        blank_bit_depth = mp_obj_get_int(dit_depth);
    }

    // Calculate number of bytes in full blank TextureResource
    // in .data section
    uint32_t unpadded_bytes_stride = 0;
    get_bit_depth_strides(blank_bit_depth, blank_width, &unpadded_bytes_stride, &self->pixel_stride);
    blank_data_bytes_size = unpadded_bytes_stride * blank_height;
    
    // Create MicroPython .data bytearray
    mp_obj_array_t *data = mp_const_none;
    mp_obj_array_t *colors = mp_const_none;

    data = m_new_obj(mp_obj_array_t);
    data->base.type = &mp_type_bytearray;
    data->typecode = BYTEARRAY_TYPECODE;
    data->free = 0;
    data->len = blank_data_bytes_size;
    data->items = m_new(byte, data->len);
    memset(data->items, 0, data->len);

    // Create MicroPython .color bytearray
    if(blank_bit_depth < 16){
        colors = m_new_obj(mp_obj_array_t);
        colors->base.type = &mp_type_bytearray;
        colors->typecode = BYTEARRAY_TYPECODE;
        colors->free = 0;
        colors->len = (1 << blank_bit_depth) * 2; // Raise two to the power of bit-depth for number of colors and multiply by 2 for 16-bit RGB565
        colors->items = m_new(byte, colors->len);
        memset(colors->items, 0, colors->len);
    }

    // Fill bitmap bytearray with initial color
    if(blank_bit_depth < 16){
        // All of the color indices are already 0, so just make the first color
        // entry in color index array the color they passed to fill the entire image
        memcpy(colors->items, &blank_color, 2);
        self->get_pixel = texture_resource_get_indexed_pixel;
    }else{
        uint16_t *pixels = data->items;

        for(uint16_t ipx=0; ipx<blank_pixel_count; ipx++){
            pixels[ipx] = blank_color;
        }

        self->get_pixel = texture_resource_get_16bit_rgb565;
    }

    self->width = blank_width;
    self->height = blank_height;
    self->data = data;
    self->colors = colors;
    self->bit_depth = blank_bit_depth;
    self->red_mask   = 0b1111100000000000;
    self->green_mask = 0b0000011111100000;
    self->blue_mask  = 0b0000000000011111;
    self->alpha_mask = 0b0000000000000000;
}


// Depending on the sign of the height of the image, need to flip the image in each case below
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfo#:~:text=If%20the%20height%20of%20the%20bitmap%20is%20positive

void copy_and_flip(texture_resource_class_obj_t *self, uint32_t pixel_data_start, uint32_t padded_width, uint32_t unpadded_bytes_width){
    // Fetch pixel data from filesystem row by row from bottom to top (flip)
    for(int32_t y=self->height-1; y>=0; y--){
        // Calculate and seek to start of each row
        uint32_t offset = y * padded_width + 0;
        engine_file_seek(0, pixel_data_start + offset, MP_SEEK_SET);
        
        chunked_read_and_store_row(unpadded_bytes_width);
    }
}


void create_from_file(texture_resource_class_obj_t *self, mp_obj_t filepath, mp_obj_t in_ram){
    // Set flag indicating if file data is to be stored in
    // ram or not (faster if stored in ram, up to programmer)
    self->in_ram = mp_obj_get_int(in_ram);

    // BMP parsing: https://en.wikipedia.org/wiki/BMP_file_format
    // https://learn.microsoft.com/en-us/windows/win32/gdi/bitmap-storage
    // Variable names are from https://github.com/WerWolv/ImHex patterns
    engine_file_open_read(0, filepath);
    engine_file_seek(0, 0, MP_SEEK_SET);

    // Basic information we need about the bitmap
    bmfh_t header;
    bmih_v1_t info_v1;
    bmih_v2_t info_v2;
    bmih_v3_t info_v3;

    // Set memory to zeros
    memset(&header, 0, sizeof(bmfh_t));
    memset(&info_v1, 0, sizeof(bmih_v1_t));
    memset(&info_v2, 0, sizeof(bmih_v2_t));
    memset(&info_v3, 0, sizeof(bmih_v3_t));

    self->pixel_stride = 0;

    // Fill header and info structs and get minimum version supported
    uint8_t version = bitmap_get_header_and_info(&header, &info_v1, &info_v2, &info_v3);

    uint32_t data_offset = sizeof(bmfh_t) + info_v1.bi_size;    // Offset to start of color table or pixel data after 14 bytes `bmfh` section and variable `bmih` section
    uint32_t color_table_size_in_file = 0;                      // Number of bytes the color table is using in the file
    uint16_t color_count = 0;                                   // Number of 16-bit colors we will need (need to calculate this, not all bitmaps have the clr_used field filled out)
    uint16_t color_table_size = 0;                              // Number of bytes needed to store all the 16-bit colors

    info_v1.bi_size_image = header.bf_size - header.bf_off_bits;                            // Not all exporters fill out `bi_size_image`, calculate it instead
    if(info_v1.bi_bit_count < 16){
        color_table_size_in_file = header.bf_size - (data_offset + info_v1.bi_size_image);  // If indexed bitmap, calculate size of file color index table (consists of u32s)
        color_count = color_table_size_in_file / 4;                                         // Number of colors in color table (might not use all available, so calculate it)
        color_table_size = color_count * 2;                                                 // How many bytes we need to store for 16-bit versions of these colors
    }else if(info_v1.bi_compression == BI_RGB){
        // According to // https://web.archive.org/web/20221228185041/https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader#:~:text=For%2016%2Dbpp%20bitmaps%2C%20if%20biCompression%20equals%20BI_RGB
        // if the compression is BI_RGB then pixel data is ALWAYS stored as 16 bit 555. If the compression is BI_BITFIELDS, then it has masks and should be resolved later
        self->red_mask   = 0b0111110000000000;
        self->green_mask = 0b0000001111100000;
        self->blue_mask  = 0b0000000000011111;
    }

    if(version >= 2){
        self->red_mask = info_v2.bi_red_mask;
        self->green_mask = info_v2.bi_green_mask;
        self->blue_mask = info_v2.bi_blue_mask;
    }

    if(version >= 3){
        self->alpha_mask = info_v3.bi_alpha_mask;
    }

    // Combine the masks into one to make it easy to check if the
    // bitmap contains a certain type of pixel data
    self->combined_masks = 0;
    self->combined_masks |= self->red_mask;
    self->combined_masks |= self->green_mask;
    self->combined_masks |= self->blue_mask;
    self->combined_masks |= self->alpha_mask;

    // Print information
    ENGINE_INFO_PRINTF("TextureResource: BMP parameters parsed from '%s':\n", mp_obj_str_get_str(filepath));
    ENGINE_INFO_PRINTF("\t min version: \t\t\t%d\n", version);
    ENGINE_INFO_PRINTF("\t bf_size: \t\t\t%d\n", header.bf_size);
    ENGINE_INFO_PRINTF("\t bf_off_bits: \t\t\t%lu\n", header.bf_off_bits);
    ENGINE_INFO_PRINTF("\t bi_size: \t\t\t%lu\n", info_v1.bi_size);
    ENGINE_INFO_PRINTF("\t bi_width: \t\t\t%lu\n", info_v1.bi_width);
    ENGINE_INFO_PRINTF("\t bi_height: \t\t\t%lu\n", info_v1.bi_height);
    ENGINE_INFO_PRINTF("\t bi_bit_count: \t\t\t%lu\n", info_v1.bi_bit_count);
    ENGINE_INFO_PRINTF("\t bi_size_image: \t\t%lu\n", info_v1.bi_size_image);

    ENGINE_INFO_PRINTF("\t bi_red_mask: \t\t\t"); print_binary(info_v2.bi_red_mask, 32); ENGINE_INFO_PRINTF("\n");
    ENGINE_INFO_PRINTF("\t bi_green_mask: \t\t"); print_binary(info_v2.bi_green_mask, 32); ENGINE_INFO_PRINTF("\n");
    ENGINE_INFO_PRINTF("\t bi_blue_mask: \t\t\t"); print_binary(info_v2.bi_blue_mask, 32); ENGINE_INFO_PRINTF("\n");
    ENGINE_INFO_PRINTF("\t bi_alpha_mask: \t\t"); print_binary(info_v3.bi_alpha_mask, 32); ENGINE_INFO_PRINTF("\n");
    ENGINE_INFO_PRINTF("\t combined_masks: \t\t%lu\n", self->combined_masks);

    ENGINE_INFO_PRINTF("\t data_offset: \t\t\t%lu\n", data_offset);
    ENGINE_INFO_PRINTF("\t color_table_size_in_file: \t%lu\n", color_table_size_in_file);
    ENGINE_INFO_PRINTF("\t color_count: \t\t\t%lu\n", color_count);
    ENGINE_INFO_PRINTF("\t color_table_size: \t\t%lu\n", color_table_size);

    // Seek to color table or pixel data (might be the same as bf_off_bits in some cases)
    engine_file_seek(0, data_offset, MP_SEEK_SET);

    // For bit-depths below 16 bits, the colors are stored
    // in a color table. The color table is 24-bits in the
    // file but will be converted to 16-bit RGB 565 so that
    // copying to the screen buffer is faster
    if(info_v1.bi_bit_count < 16){
        mp_obj_array_t *colors = engine_resource_get_space_bytearray(color_table_size, true);
        bitmap_get_and_fill_color_table((uint16_t*)colors->items, color_count);
        self->colors = colors;

        // The bitmasks changed to RGB565
        self->red_mask   = 0b1111100000000000;
        self->green_mask = 0b0000011111100000;
        self->blue_mask  = 0b0000000000011111;
    }else{
        self->colors = mp_const_none;   // No color table for higher than 8 bit-depths
    }

    // Now that we know the bitmap information, fill out some
    // of the `TextureResource` attributes
    self->width = info_v1.bi_width;
    self->height = info_v1.bi_height;
    self->bit_depth = info_v1.bi_bit_count;

    // Figure out the number of bytes in each row of the image in the file
    uint32_t padded_bytes_width = 0;

    // https://en.wikipedia.org/wiki/BMP_file_format#:~:text=Each%20row%20in%20the%20Pixel%20array%20is%20padded%20to%20a%20multiple%20of%204%20bytes%20in%20size
    uint32_t unpadded_bytes_width = 0;
    get_bit_depth_strides(self->bit_depth, self->width, &unpadded_bytes_width, &self->pixel_stride);

    // Pad to 4 bytes
    padded_bytes_width = (uint32_t)(ceilf((float)unpadded_bytes_width / 4.0f) * 4.0f);

    // Figure out the total space in RAM or FLASH scratch to allocate
    // for the final image data.
    uint32_t total_required_space = 0;

    if(self->bit_depth < 16){
        // Images using indexed colors have their index data copied
        // directly to the .data space in RAM or FLASH
        total_required_space = unpadded_bytes_width * self->height;
    }else{
        // Not any of the other case, must be 16-bit image which will
        // get its pixel data directly copied to RAM or FLASH even if
        // it contains alpha bits in the color masks (decoded on the fly)
        uint32_t pixel_count = self->width * self->height;
        total_required_space = pixel_count*2;
    }

    // Allocate the space and start storing process
    self->data = engine_resource_get_space_bytearray(total_required_space, self->in_ram);
    engine_resource_start_storing(self->data, self->in_ram);

    // All pixels are directly copied without modification
    // for 1 ~ 16 bit bitmaps
    copy_and_flip(self, header.bf_off_bits, padded_bytes_width, unpadded_bytes_width);

    // Close reading file and stop storing in resource space
    engine_file_close(0);
    engine_resource_stop_storing();

    // Assign a function for getting pixels from texture resource
    if(self->bit_depth < 16){
        self->get_pixel = texture_resource_get_indexed_pixel;
    }else{
        if((self->combined_masks == 65535 && self->alpha_mask == 0) || self->combined_masks == 0){   // RGB565
            self->get_pixel = texture_resource_get_16bit_rgb565;
        }else{
            self->get_pixel = texture_resource_get_16bit_axrgb;

            // See: `texture_resource_get_16bit_argb`
            // Each channel needs to be shifted all the way to the right.
            // Need to calculate how many bits are to the right of each channel
            // mask (r_mask, g_mask, etc.). To do this, figure how the number that
            // encompass the mask bits and the bit to the right (always a prw of 2 - 1)
            // Since a^b=c, b=log_a(c): https://math.stackexchange.com/a/673801
            //
            //  Continued example:  a_all_right = 2^ceil(log2(a_mask))-1 = 2^ceil(log2(0b1_00000_00000_00000 + 1))-1 = 2^ceil(log2(32768 + 1))-1 = 2^ceil(15.00004) - 1 = (2^16)-1 = 65535 = 0b1111111111111111
            //                      r_all_right = 2^ceil(log2(r_mask))-1 = 2^ceil(log2(0b0_11111_00000_00000 + 1))-1 = 2^ceil(log2(31744 + 1))-1 = 2^ceil(14.954) - 1   = (2^15)-1 = 32767 = 0b0111111111111111
            //                      g_all_right = 2^ceil(log2(g_mask))-1 = 2^ceil(log2(0b0_00000_11111_00000 + 1))-1 = 2^ceil(log2(992 + 1))-1   = 2^ceil(9.9556) - 1   = (2^10)-1 = 2047  = 0b0000001111111111
            //                      not needed for blue, already in right-most bits
            //
            // Add one so that rounding up is always to the next int
            uint16_t a_all_right = (uint16_t)powf(2, ceilf(log2f(self->alpha_mask+1))) - 1;
            uint16_t r_all_right = (uint16_t)powf(2, ceilf(log2f(self->red_mask+1))) - 1;
            uint16_t g_all_right = (uint16_t)powf(2, ceilf(log2f(self->green_mask+1))) - 1;

            // The above masks include the bits of the color channel mask, subtract that
            // mask out of the `all_right` masks
            //
            //  Continued example:  a_just_right = a_all_right - a_mask = 0b1111111111111111 - 0b1_00000_00000_00000 = 0b0111111111111111 = 32767
            //                      r_just_right = r_all_right - r_mask = 0b0111111111111111 - 0b0_11111_00000_00000 = 0b0000001111111111 = 1023
            //                      g_just_right = g_all_right - g_mask = 0b0000001111111111 - 0b0_00000_11111_00000 = 0b0000000000011111 = 31
            //                      not needed for blue, already in right-most bits, would be 0
            uint16_t a_just_right = a_all_right - self->alpha_mask;
            uint16_t r_just_right = r_all_right - self->red_mask;
            uint16_t g_just_right = g_all_right - self->green_mask;

            // Using the bits that are just to the right of each color channel mask, calculate
            // how many bits there are:
            //
            //  Continued example:  a_right_shift_amount = ceil(log2(a_just_right)) = ceil(log2(32767)) = ceil(14.99996) = 15
            //                      r_right_shift_amount = ceil(log2(r_just_right)) = ceil(log2(1023))  = ceil(9.999)    = 10
            //                      g_right_shift_amount = ceil(log2(g_just_right)) = ceil(log2(31))    = ceil(4.954)    = 5
            //                      not needed for blue, already in right-most bits, would be 0
            self->a_mask_right_shift_amount = (uint16_t)ceilf(log2f(a_just_right));
            self->r_mask_right_shift_amount = (uint16_t)ceilf(log2f(r_just_right));
            self->g_mask_right_shift_amount = (uint16_t)ceilf(log2f(g_just_right));

            // Now that the bits for each channel are all the way to the right, need
            // to shift them so that the bits are in the left/high side of the channel
            // of the RGB565 channel (except for alpha)
            //
            //  Continued example:  r_right_shift_amount -= ceil(log2(0b00011111)) - ceil(log2(r_mask >> r_right_shift_amount)) -= ceil(log2(31)) - ceil(log2(0b0_11111_00000_00000 >> 10)) -= 5 - ceil(log2(31)) -= 5 - 5 -= 0
            //                      g_right_shift_amount -= ceil(log2(0b00111111)) - ceil(log2(g_mask >> g_right_shift_amount)) -= ceil(log2(63)) - ceil(log2(0b0_00000_11111_00000 >> 5))  -= 6 - ceil(log2(31)) -= 6 - 5 -= 1
            //           special -> b_left_shift_amount -= ceil(log2(0b00011111)) - ceil(log2(b_mask))                          -= ceil(log2(31)) - ceil(log2(0b0_00000_00000_11111))       -= 5 - ceil(log2(31)) -= 5 - 5 -= 0
            //
            //  Blue channel is already all the right, need to shift it left to get it into the RGB565 hi bits
            self->r_mask_right_shift_amount -= (uint16_t)(ceilf(log2f(0b00011111)) - ceilf(log2f(self->red_mask >> self->r_mask_right_shift_amount)));
            self->g_mask_right_shift_amount -= (uint16_t)(ceilf(log2f(0b00111111)) - ceilf(log2f(self->green_mask >> self->g_mask_right_shift_amount)));
            self->b_mask_left_shift_amount   = (uint16_t)(ceilf(log2f(0b00011111)) - ceilf(log2f(self->blue_mask)));
        }
    }
}


mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TextureResource");

    texture_resource_class_obj_t *self = mp_obj_malloc_with_finaliser(texture_resource_class_obj_t, &texture_resource_class_type);
    self->base.type = &texture_resource_class_type;

    switch(n_args){
        case 1: // File path
        {
            if(mp_obj_is_str(args[0]) == false){
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Expected file path `str`, got: %s"), mp_obj_get_type_str(args[0]));
            }

            // If not specified, not in ram by default
            create_from_file(self, args[0], mp_const_false);
        }
        break;
        case 2: // `file_path` and `in_ram` or `width` and `height`
        {
            if(mp_obj_is_str(args[0]) && mp_obj_is_bool(args[1])){
                create_from_file(self, args[0], args[1]);
            }else if(mp_obj_is_int(args[0]) && mp_obj_is_int(args[1])){
                create_blank_from_params(self, args[0], args[1], mp_const_none, mp_const_none);
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Expected file path `str` and in_ram `bool` or width `int` and height `int`, got: %s %s"), mp_obj_get_type_str(args[0]), mp_obj_get_type_str(args[1]));
            }
        }
        break;
        case 3: // `width`, `height`, and `color`
        {
            if(mp_obj_is_int(args[0]) && mp_obj_is_int(args[1]) && (mp_obj_is_int(args[2]) || mp_obj_is_type(args[2], &const_color_class_type) || mp_obj_is_type(args[2], &color_class_type))){
                create_blank_from_params(self, args[0], args[1], args[2], mp_const_none);
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Expected width `int`, height `int`, and `int` | `const_color` | `color` got: %s %s %s"), mp_obj_get_type_str(args[0]), mp_obj_get_type_str(args[1]), mp_obj_get_type_str(args[2]));
            }
        }
        break;
        case 4: // `width`, `height`, `color`, and `bit_depth`
        {
            if(mp_obj_is_int(args[0]) && mp_obj_is_int(args[1]) && (mp_obj_is_int(args[2]) || mp_obj_is_type(args[2], &const_color_class_type) || mp_obj_is_type(args[2], &color_class_type)) && mp_obj_is_int(args[3])){
                create_blank_from_params(self, args[0], args[1], args[2], args[3]);
            }else{
                mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Expected width `int`, height `int`, `int` | `const_color` | `color`, and `int` got: %s %s %s %s"), mp_obj_get_type_str(args[0]), mp_obj_get_type_str(args[1]), mp_obj_get_type_str(args[2]), mp_obj_get_type_str(args[3]));
            }
        }
        break;
        default:
        {
            mp_raise_msg_varg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Expected 1 ~ 4 arguments, got: %d"), n_args);
        }
    }
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
static mp_obj_t texture_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("TextureResource: Deleted");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(texture_resource_class_del_obj, texture_resource_class_del);


/*  --- doc ---
    NAME: TextureResource
    ID: TextureResource
    DESC: Object that holds pixel information. If a file path is specifed, the bitmap needs to be a 16-bit or less format. If at least a width and height are specified instead, a blank white RGB565 texture is created in RAM but an initial color can also be passed. If a `bit_depth` is passed, the first entry in the color table will be set to `color` and the entire blank image will index to that.
    PARAM:  [type=string | int]     [name=filepath | width]     [value=string | 0 ~ 65535]
    PARAM:  [type=bool | int]       [name=in_ram   | height]    [value=True or False (default: False) | 0 ~ 65535]
    PARAM:  [type=int]              [name=color]                [value=int 16-bit RGB565 (optional)]
    PARAM:  [type=int]              [name=bit_depth]            [value=1, 4, 8, or 16 (optional)]

    ATTR:   [type=float]            [name=width]                [value=any (read-only)]
    ATTR:   [type=float]            [name=height]               [value=any (read-only)]
    ATTR:   [type=int]              [name=bit_depth]            [value=any (read-only)]
    ATTR:   [type=int]              [name=red_mask]             [value=any (read-only)]
    ATTR:   [type=int]              [name=blue_mask]            [value=any (read-only)]
    ATTR:   [type=int]              [name=green_mask]           [value=any (read-only)]
    ATTR:   [type=int]              [name=alpha_mask]           [value=any (read-only)]
    ATTR:   [type=bytearray]        [name=data]                 [value=RGB565 bytearray (note, if in_ram is False, then writing to this is not a valid operation)]
    ATTR:   [type=bytearray]        [name=colors]               [value=RGB565 bytearray (when the bit-depth is less than 16, this will be filled with RGB565 converted colors)]
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
            case MP_QSTR_bit_depth:
                destination[0] = mp_obj_new_int(self->bit_depth);
            break;
            case MP_QSTR_red_mask:
                destination[0] = mp_obj_new_int(self->red_mask);
            break;
            case MP_QSTR_green_mask:
                destination[0] = mp_obj_new_int(self->green_mask);
            break;
            case MP_QSTR_blue_mask:
                destination[0] = mp_obj_new_int(self->blue_mask);
            break;
            case MP_QSTR_alpha_mask:
                destination[0] = mp_obj_new_int(self->alpha_mask);
            break;
            case MP_QSTR_colors:
                destination[0] = self->colors;
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
            case MP_QSTR_colors:
            {
                mp_obj_array_t *new_bytearray = destination[1];
                mp_obj_array_t *cur_bytearray = self->colors;
                if(cur_bytearray->len != new_bytearray->len){
                    mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Can't set texture colors to new bytearray, lengths do not match!"));
                }
                self->colors = destination[1];
            }
            break;
            case MP_QSTR_bit_depth:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Bit depth of a texture cannot be set!"));
            break;
            case MP_QSTR_red_mask:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Red mask of a texture cannot be set!"));
            break;
            case MP_QSTR_green_mask:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Green mask of a texture cannot be set!"));
            break;
            case MP_QSTR_blue_mask:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Blue mask of a texture cannot be set!"));
            break;
            case MP_QSTR_alpha_mask:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: ERROR: Alpha mask of a texture cannot be set!"));
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