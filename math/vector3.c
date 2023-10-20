#include "vector3.h"
#include "utility/debug_print.h"

// Class required functions
STATIC void vector3_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): Vector3]");
}

STATIC mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Vector3");
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    vector3_class_obj_t *self = m_new_obj(vector3_class_obj_t);
    self->base.type = &vector3_class_type;

    self->x = 0.0f;
    self->y = 0.0f;
    self->z = 0.0f;

    return MP_OBJ_FROM_PTR(self);
}


// Class methods

STATIC mp_obj_t vector3_class_test(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("Vector3 test");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_test_obj, vector3_class_test);


// Class attributes
STATIC const mp_rom_map_elem_t vector3_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_test), MP_ROM_PTR(&vector3_class_test_obj) },
};

// Class init
STATIC MP_DEFINE_CONST_DICT(vector3_class_locals_dict, vector3_class_locals_dict_table);

const mp_obj_type_t vector3_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Vector3,
    .print = vector3_class_print,
    .make_new = vector3_class_new,
    .locals_dict = (mp_obj_dict_t*)&vector3_class_locals_dict,
};