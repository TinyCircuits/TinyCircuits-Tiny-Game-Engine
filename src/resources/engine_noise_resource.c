#define FNL_IMPL

#include "engine_noise_resource.h"
#include "debug/debug_print.h"
#include <stdlib.h>

// Class required functions
STATIC void noise_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): NoiseResource");
}


mp_obj_t noise_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New NoiseResource");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    noise_resource_class_obj_t *self = m_new_obj_with_finaliser(noise_resource_class_obj_t);
    self->base.type = &noise_resource_class_type;

    self->fnl = fnlCreateState();
    
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t noise_resource_class_noise_3d(size_t n_args, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("NoiseResource: Deleted (freeing noise data)");

    noise_resource_class_obj_t *self = args[0];

    float x = mp_obj_get_float(args[1]);
    float y = mp_obj_get_float(args[2]);
    float z = mp_obj_get_float(args[3]);

    return mp_obj_new_float(fnlGetNoise3D(&self->fnl, x, y, z));
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(noise_resource_class_noise_3d_obj, 4, 4, noise_resource_class_noise_3d);


STATIC mp_obj_t noise_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("NoiseResource: Deleted (freeing noise data)");

    noise_resource_class_obj_t *self = self_in;

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(noise_resource_class_del_obj, noise_resource_class_del);


STATIC void noise_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing NoiseResource attr");

    noise_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&noise_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_noise_3d:
                destination[0] = MP_OBJ_FROM_PTR(&noise_resource_class_noise_3d_obj);
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
STATIC const mp_rom_map_elem_t noise_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(noise_resource_class_locals_dict, noise_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    noise_resource_class_type,
    MP_QSTR_NoiseResource,
    MP_TYPE_FLAG_NONE,

    make_new, noise_resource_class_new,
    print, noise_resource_class_print,
    attr, noise_resource_class_attr,
    locals_dict, &noise_resource_class_locals_dict
);