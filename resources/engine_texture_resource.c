#include "engine_texture_resource.h"
#include "debug/debug_print.h"


// Class required functions
STATIC void texture_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): TextureResource");
}


mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TextureResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    texture_resource_class_obj_t *self = m_new_obj_with_finaliser(texture_resource_class_obj_t);

    // mp_file_t *file = mp_open(mp_obj_str_get_str(args[0]), "rb");
    self->width = mp_obj_new_int(32);
    self->height = mp_obj_new_int(32);

    // uint8_t buf[256];

    // const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + 512*1024);

    // for(uint8_t i=0; i<8; i++){
    //     flash_range_erase((512*1024)+(i*4096), FLASH_SECTOR_SIZE);
    // }

    // uint32_t index=0;
    // while(mp_readinto(file, buf, 256) != 0){
    //     flash_range_program((512*1024)+index, buf, 256);
    //     index += 256;
    // }

    // mp_close(file);

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t texture_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("TextureResource: Deleted (freeing texture data)");

    texture_resource_class_obj_t *self = self_in;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(texture_resource_class_del_obj, texture_resource_class_del);


mp_obj_t texture_resource_get_pixel(mp_obj_t self_in, mp_obj_t x_in, mp_obj_t y_in){
    ENGINE_INFO_PRINTF("TextureResource: Getting pixel");

    // texture_resource_class_obj_t *self = self_in;
    // mp_int_t x = mp_obj_get_int(x_in);
    // mp_int_t y = mp_obj_get_int(y_in);
    // mp_int_t width = mp_obj_get_int(self->width);

    // mp_seek(self->file, y*width+x, MP_SEEK_SET);

    // return mp_obj_new_int(mp_readbyte_u16(self->file));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(texture_resource_get_pixel_obj, texture_resource_get_pixel);


STATIC void texture_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing TextureResource attr");

    texture_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&texture_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_get_pixel:
                destination[0] = MP_OBJ_FROM_PTR(&texture_resource_get_pixel_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_width:
                destination[0] = self->width;
            break;
            case MP_QSTR_height:
                destination[0] = self->height;
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