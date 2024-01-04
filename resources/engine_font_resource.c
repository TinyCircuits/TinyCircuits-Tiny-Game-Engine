#include "engine_font_resource.h"
#include "debug/debug_print.h"


// Class required functions
STATIC void font_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): FontResource");
}


mp_obj_t font_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New FontResource");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    font_resource_class_obj_t *self = m_new_obj_with_finaliser(font_resource_class_obj_t);
    self->base.type = &font_resource_class_type;
    // self->filename = mp_obj_str_get_str(args[0]);
    // self->font_size = 12.0f;

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t font_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("FontResource: Deleted");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(font_resource_class_del_obj, font_resource_class_del);


STATIC void font_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing FontResource attr");

    font_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&font_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_filename:
                destination[0] = self->filename;
            break;
            case MP_QSTR_font_size:
                destination[0] = mp_obj_new_float(self->font_size);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_filename:
                self->filename = destination[1];
            break;
            case MP_QSTR_font_size:
                self->font_size = mp_obj_get_float(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t font_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(font_resource_class_locals_dict, font_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    font_resource_class_type,
    MP_QSTR_FontResource,
    MP_TYPE_FLAG_NONE,

    make_new, font_resource_class_new,
    print, font_resource_class_print,
    attr, font_resource_class_attr,
    locals_dict, &font_resource_class_locals_dict
);