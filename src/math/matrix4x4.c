#include "matrix4x4.h"
#include "debug/debug_print.h"


mp_obj_t matrix4x4_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Matrix4x4");
    matrix4x4_class_obj_t *self = m_new_obj(matrix4x4_class_obj_t);
    self->base.type = &matrix4x4_class_type;

    for(uint8_t i=0; i<16; i++){
        for(uint8_t j=0; j<4; j++){
            self->m[i][j].base.type = &mp_type_float;
        }
    }
    
    return MP_OBJ_FROM_PTR(self);
}


STATIC void matrix4x4_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Matrix4x4 attr");

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
STATIC const mp_rom_map_elem_t matrix4x4_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(matrix4x4_class_locals_dict, matrix4x4_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    matrix4x4_class_type,
    MP_QSTR_Matrix4x4,
    MP_TYPE_FLAG_NONE,

    make_new, matrix4x4_class_new,
    attr, matrix4x4_class_attr,
    locals_dict, &matrix4x4_class_locals_dict
);