#include "engine_texture_resource.h"
#include "debug/debug_print.h"
#include "resources/engine_resource_manager.h"
#include <stdlib.h>


// Class required functions
STATIC void texture_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): TextureResource");
}


mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TextureResource");
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    texture_resource_class_obj_t *self = m_new_obj_with_finaliser(texture_resource_class_obj_t);
    self->base.type = &texture_resource_class_type;

    // Set flag indicating if file data is to be stored in ram or not (faster if stored in ram, up to programmer)
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
    engine_file_open(mp_obj_str_get_str(args[0]));

    uint16_t bitmap_id = engine_file_get_u16(0);
    uint32_t bitmap_pixel_data_offset = engine_file_get_u32(10);
    uint32_t bitmap_width = engine_file_get_u32(18);
    uint32_t bitmap_height = engine_file_get_u32(22);
    uint32_t bitmap_bits_per_pixel = engine_file_get_u16(28);
    uint32_t bitmap_data_size = engine_file_get_u32(34);

    ENGINE_INFO_PRINTF("TextureResource: BMP Parameters parsed from '%s':", mp_obj_str_get_str(args[0]));
    ENGINE_INFO_PRINTF("\tbitmap_id:\t\t\t%d", bitmap_id);
    ENGINE_INFO_PRINTF("\tbitmap_pixel_data_offset:\t%lu", bitmap_pixel_data_offset);
    ENGINE_INFO_PRINTF("\tbitmap_width:\t\t\t%lu", bitmap_width);
    ENGINE_INFO_PRINTF("\tbitmap_height:\t\t\t%lu", bitmap_height);
    ENGINE_INFO_PRINTF("\tbitmap_bits_per_pixel:\t\t%lu", bitmap_bits_per_pixel);
    ENGINE_INFO_PRINTF("\tbitmap_data_size:\t\t%lu", bitmap_data_size);

    // Check header ID field
    if(bitmap_id != 19778){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP header ID field incorrect!"));
    }

    if(bitmap_bits_per_pixel != 16){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("TextureResource: BMP has wrong color depth! Expects 16-bits per pixel"));
    }

    self->width = bitmap_width;
    self->height = bitmap_height;
    
    // Pass the size of the pixel data, and if it is in fast ram or not
    self->data = (uint16_t*)engine_resource_get_space(bitmap_data_size, self->in_ram);
    engine_resource_start_storing((uint8_t*)self->data, self->in_ram);

    uint16_t bitmap_pixel_src_x = 0;
    uint16_t bitmap_pixel_src_y = bitmap_height-1;
    for(uint32_t bitmap_pixel_dest_index=0; bitmap_pixel_dest_index<bitmap_width*bitmap_height; bitmap_pixel_dest_index++){
        
        uint32_t bitmap_pixel_src_index = bitmap_pixel_src_y * bitmap_width + bitmap_pixel_src_x;
        engine_resource_store_u16(engine_file_get_u16(bitmap_pixel_data_offset+bitmap_pixel_src_index*2));

        bitmap_pixel_src_x++;
        if(bitmap_pixel_src_x >= bitmap_width){
            bitmap_pixel_src_x = 0;
            bitmap_pixel_src_y--;
        }
    }

    engine_resource_stop_storing();
    engine_file_close();
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t texture_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("TextureResource: Deleted (freeing texture data)");

    texture_resource_class_obj_t *self = self_in;

    if(self->in_ram){
        #if defined(__unix__)
            free(self->data);
        #elif (__arm__)
            m_free(self->data);
        #else
            #error "TextureResource: Unknown platform"
        #endif
    }else{
        // Nothing done if not in ram, that flash goes to waste, for now
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(texture_resource_class_del_obj, texture_resource_class_del);


uint16_t texture_resource_get_pixel(texture_resource_class_obj_t *texture, uint32_t offset){
    return texture->data[offset];
}


STATIC void texture_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
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
STATIC const mp_rom_map_elem_t texture_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(texture_resource_class_locals_dict, texture_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    texture_resource_class_type,
    MP_QSTR_TextureResource,
    MP_TYPE_FLAG_NONE,

    make_new, texture_resource_class_new,
    print, texture_resource_class_print,
    attr, texture_resource_class_attr,
    locals_dict, &texture_resource_class_locals_dict
);