#include "vector3.h"
#include "debug/debug_print.h"

// Class required functions
STATIC void vector3_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    vector3_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): Vector3 [%0.3f, %0.3f, %0.3f]", (double)self->x.value, (double)self->y.value, (double)self->z.value);
}


mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Vector3");
    vector3_class_obj_t *self = m_new_obj(vector3_class_obj_t);
    self->base.type = &vector3_class_type;

    self->x.base.type = &mp_type_float;
    self->y.base.type = &mp_type_float;
    self->z.base.type = &mp_type_float;

    if(n_args == 0) {
      
      self->x.value = 0.0f;
      self->y.value = 0.0f;
      self->z.value = 0.0f;
    } else if(n_args == 3) {
      self->x.value = mp_obj_get_float(args[0]);
      self->y.value = mp_obj_get_float(args[1]);
      self->z.value = mp_obj_get_float(args[2]);
    }else{
      mp_raise_TypeError(MP_ERROR_TEXT("function takes 0 or 3 arguments"));
    }
    ENGINE_INFO_PRINTF("Returning Vector3");
    return MP_OBJ_FROM_PTR(self);
}


/*  --- doc ---
    NAME: Vector3
    DESC: Holds an X, Y, and Z value. Typically used for position
    PARAM: [type=float]  [name=x]    [value=any]
    PARAM: [type=float]  [name=y]    [value=any]
    PARAM: [type=float]  [name=z]    [value=any]                                                                                      
    ATTR: [type=float]  [name=x]    [value=any]                                
    ATTR: [type=float]  [name=y]    [value=any]
    ATTR: [type=float]  [name=z]    [value=any]                                                               
*/ 
STATIC void vector3_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Vector3 attr");

    vector3_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_x:
                destination[0] = &self->x;
            break;
            case MP_QSTR_y:
                destination[0] = &self->y;
            break;
            case MP_QSTR_z:
                destination[0] = &self->z;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_x:
                self->x.value = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_y:
                self->y.value = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_z:
                self->z.value = mp_obj_get_float(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t vector3_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(vector3_class_locals_dict, vector3_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    vector3_class_type,
    MP_QSTR_Vector3,
    MP_TYPE_FLAG_NONE,

    make_new, vector3_class_new,
    print, vector3_class_print,
    attr, vector3_class_attr,
    locals_dict, &vector3_class_locals_dict
);