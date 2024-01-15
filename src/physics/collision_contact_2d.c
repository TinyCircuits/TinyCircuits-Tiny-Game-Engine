#include "collision_contact_2d.h"


// Class required functions
STATIC void collision_contact_2d_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): CollisionContact2D");
}


mp_obj_t collision_contact_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New CollisionContact2D");
    mp_arg_check_num(n_args, n_kw, 0, 5, false);

    collision_contact_2d_class_obj_t *self = m_new_obj(collision_contact_2d_class_obj_t);
    self->base.type = &collision_contact_2d_class_type;

    if(n_args == 0){
        self->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
        self->normal = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    }else if(n_args == 5){
        mp_obj_t parameters[2];
        parameters[0] = args[0];
        parameters[1] = args[1];
        self->position = vector2_class_new(&vector2_class_type, 0, 0, parameters);

        parameters[0] = args[2];
        parameters[1] = args[3];
        self->normal = vector2_class_new(&vector2_class_type, 0, 0, parameters);

        self->node = args[4];
    }else{
        mp_raise_msg(MP_QSTR_RuntimeError, MP_ERROR_TEXT("CollisionContact2D: Expected 0 or 4 arguments, got something else..."));
    }
    
    return MP_OBJ_FROM_PTR(self);
}


STATIC void collision_contact_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing CollisionContact2D attr");

    collision_contact_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_normal:
                destination[0] = self->normal;
            break;
            case MP_QSTR_node:
                destination[0] = self->node;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_normal:
                self->normal = destination[1];
            break;
            case MP_QSTR_node:
                self->node = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t collision_contact_2d_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(collision_contact_2d_class_locals_dict, collision_contact_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    collision_contact_2d_class_type,
    MP_QSTR_CollisionContact2D,
    MP_TYPE_FLAG_NONE,

    make_new, collision_contact_2d_class_new,
    print, collision_contact_2d_class_print,
    attr, collision_contact_2d_class_attr,
    locals_dict, &collision_contact_2d_class_locals_dict
);