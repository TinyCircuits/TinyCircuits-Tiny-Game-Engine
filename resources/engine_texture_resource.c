#include "engine_texture_resource.h"
#include "debug/debug_print.h"

#include "libs/mpfile/mpfile.h"

// https://raw.githubusercontent.com/nothings/stb/beebb24b945efdea3b9bba23affb8eb3ba8982e7/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#define STBI_NO_STDIO   // use custom callbacks, don't need UNIX C IO stb functions
#define STBI_NO_THREAD_LOCALS
#include "libs/stb/stb_image.h"


// Wrapper around mp_file_t to keep track of file size
// for indicating if at end of file
typedef struct{
    mp_file_t *file;
    int32_t file_size;
}stb_mp_file_wrapper_t;


static int stb_custom_read_cb(void *user, char *data, int size){
    ENGINE_INFO_PRINTF("STB READ CB: Reading %d bytes", size);
    stb_mp_file_wrapper_t *stb_file_wrapper = user;

    return mp_readinto(stb_file_wrapper->file, data, size);
}


static void stb_custom_skip_cb(void *user, int n){
    ENGINE_INFO_PRINTF("STB SKIP CB: Skipping %d bytes", n);
    stb_mp_file_wrapper_t *stb_file_wrapper = user;
    mp_seek(stb_file_wrapper->file, n, MP_SEEK_CUR);
}


static int stb_custom_eof_cb(void *user){
    ENGINE_INFO_PRINTF("STB EOF CB: Checking if end of file...");
    stb_mp_file_wrapper_t *stb_file_wrapper = user;

    // https://github.com/nothings/stb/blob/03f50e343d796e492e6579a11143a085429d7f5d/stb_image.h#L413
    if(mp_tell(stb_file_wrapper->file) == stb_file_wrapper->file_size){ // EOF
        return 1;
    }else{                                                              // NOT EOF
        return 0;
    }

    return 0;
}


static const stbi_io_callbacks stb_custom_callbacks = {
    .read = stb_custom_read_cb,
    .skip = stb_custom_skip_cb,
    .eof = stb_custom_eof_cb
};


// Class required functions
STATIC void texture_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): TextureResource");
}


mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TextureResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    // Reference these in code so UNIX port doesn't complain about unused functions
    (void)&stbi__mul2shorts_valid;
    (void)&stbi__addints_valid;

    texture_resource_class_obj_t *self = m_new_obj_with_finaliser(texture_resource_class_obj_t);

    const char *filename = mp_obj_str_get_str(args[0]);
    ENGINE_INFO_PRINTF("Opening file '%s'...", filename);

    stb_mp_file_wrapper_t stb_file_wrapper;
    stb_file_wrapper.file = mp_open(filename, "rb");

    ENGINE_INFO_PRINTF("Seeking to end of '%s' to get file size", filename);
    stb_file_wrapper.file_size = mp_seek(stb_file_wrapper.file, 0, MP_SEEK_END);
    ENGINE_INFO_PRINTF("'%s' has a file size of %lu bytes", filename, stb_file_wrapper.file_size);

    ENGINE_INFO_PRINTF("Seeking back to start of '%s' to read data", filename);
    mp_seek(stb_file_wrapper.file, 0, MP_SEEK_SET);

    int width;
    int height;
    int color_component_count;  // RGB (3), RGBA (4), https://github.com/nothings/stb/blob/03f50e343d796e492e6579a11143a085429d7f5d/stb_image.h#L374-L382

    ENGINE_INFO_PRINTF("Loading image data...");
    self->texture_data = stbi_load_from_callbacks(&stb_custom_callbacks, &stb_file_wrapper.file, &width, &height, &color_component_count, 3);

    if(self->texture_data){
        ENGINE_INFO_PRINTF("Loaded image data!");

        ENGINE_INFO_PRINTF("Source image parameters ('%s'):", filename);
        ENGINE_INFO_PRINTF("\tfile size: \t%lu \t[bytes]", stb_file_wrapper.file_size);
        ENGINE_INFO_PRINTF("\twidth: \t\t%d \t[pixels]", width);
        ENGINE_INFO_PRINTF("\theight: \t%d \t[pixels]", height);
        ENGINE_INFO_PRINTF("\tcomponents: \t%d \t[number of color channels]", color_component_count);

        ENGINE_INFO_PRINTF("Loaded/in ram (C HEAP) image parameters ('%s'):", filename);
        ENGINE_INFO_PRINTF("\tin ram size: \t%lu \t[bytes (number_of_pixels(%lu) * desired_color_channels_per_pixel(%d))]", width*height*3, width*height, 3);
        ENGINE_INFO_PRINTF("\twidth: \t\t%d \t[pixels]", width);
        ENGINE_INFO_PRINTF("\theight: \t%d \t[pixels]", height);
        ENGINE_INFO_PRINTF("\tcomponents: \t%d \t[desired number of color channels]", 3);

        ENGINE_INFO_PRINTF("...Converting loaded iamge data from 3 bytes per color pixel to uint16_t 2 bytes per pixel...");

        uint16_t *data = (uint16_t*)self->texture_data;

        for(int y_scan_line=0; y_scan_line<height; y_scan_line++){
            for(int x_pixel=0; x_pixel<width; x_pixel++){
                uint32_t index_pixel = (y_scan_line * width + x_pixel);
                uint32_t index_three_channel = index_pixel*3;

                uint16_t r = self->texture_data[index_three_channel]   & 0b0000000000011111;  // 5
                uint16_t g = self->texture_data[index_three_channel+1] & 0b0000000000111111;  // 6
                uint16_t b = self->texture_data[index_three_channel+2] & 0b0000000000011111;  // 5

                uint16_t pixel = 0b0000000000000000;
                pixel = pixel | (r << 11);
                pixel = pixel | (g << 5);
                pixel = pixel | (b << 0);

                data[index_pixel] = pixel;
            }
        }

        ENGINE_INFO_PRINTF("Converted image data to RGB565, reallocating to smaller size of %lu bytes (C HEAP) (number_of_pixels*2)", width*height*2);
        self->texture_data = realloc(self->texture_data, width*height*2);

        if(self->texture_data != NULL){
            ENGINE_INFO_PRINTF("Realloc of image data successful!");
        }else{
            ENGINE_ERROR_PRINTF("TextureResource: Could not resize image data for '%s'", filename);
        }

        // Make a reference to the texture data that can be accessed in MicroPython
        self->texture_data_bytearray = mp_obj_new_bytearray_by_ref(width*height*2, (void*)self->texture_data);
    }else{
        ENGINE_ERROR_PRINTF("TextureResource: Failed loading image data for '%s'", filename);
    }

    ENGINE_INFO_PRINTF("Closing file '%s'...", filename);
    mp_close(stb_file_wrapper.file);

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t texture_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("TextureResource: Deleted (freeing texture data)");

    texture_resource_class_obj_t *self = self_in;
    free(self->texture_data);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(texture_resource_class_del_obj, texture_resource_class_del);


STATIC void texture_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing TextureResource attr");

    texture_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&texture_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_data:
                destination[0] = self->texture_data_bytearray;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t texture_resource_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(texture_resource_class_locals_dict, texture_resource_class_locals_dict_table);

const mp_obj_type_t texture_resource_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_TextureResource,
    .print = texture_resource_class_print,
    .make_new = texture_resource_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = texture_resource_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&texture_resource_class_locals_dict,
};