#include "engine_texture_resource.h"
#include "debug/debug_print.h"

#include "libs/mpfile/mpfile.h"

// https://raw.githubusercontent.com/nothings/stb/beebb24b945efdea3b9bba23affb8eb3ba8982e7/stb_image.h
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)
#define STBI_ONLY_PNG
#define STBI_NO_HDR
#include "libs/stb/stb_image.h"

// Class required functions
STATIC void texture_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): TextureResource");
}


mp_obj_t texture_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New TextureResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    (void)&stbi__mul2shorts_valid;
    (void)&stbi__addints_valid;

    texture_resource_class_obj_t *self = m_new_obj(texture_resource_class_obj_t);

    const char *filename = mp_obj_str_get_str(args[1]);
    mp_file_t *file = mp_open(filename, "rb");

    mp_close(file);

    return MP_OBJ_FROM_PTR(self);
}


STATIC void texture_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing TextureResource attr");

    // texture_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
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