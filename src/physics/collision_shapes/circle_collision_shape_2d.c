#include "circle_collision_shape_2d.h"


// Class required functions
STATIC void circle_collision_shape_2d_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): CircleCollisionShape2D");
}


mp_obj_t circle_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New CircleCollisionShape2D");
    mp_arg_check_num(n_args, n_kw, 1, 1, false);

    circle_collision_shape_2d_class_obj_t *self = m_new_obj(circle_collision_shape_2d_class_obj_t);
    self->base.type = &circle_collision_shape_2d_class_type;
    self->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    self->radius = mp_obj_get_float(args[0]);
    
    return MP_OBJ_FROM_PTR(self);
}


STATIC void circle_collision_shape_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing CircleCollisionShape2D attr");

    circle_collision_shape_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_radius:
                destination[0] = mp_obj_new_float(self->radius);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_radius:
                self->radius = mp_obj_get_float(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t circle_collision_shape_2d_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(circle_collision_shape_2d_class_locals_dict, circle_collision_shape_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    circle_collision_shape_2d_class_type,
    MP_QSTR_CircleCollisionShape2D,
    MP_TYPE_FLAG_NONE,

    make_new, circle_collision_shape_2d_class_new,
    print, circle_collision_shape_2d_class_print,
    attr, circle_collision_shape_2d_class_attr,
    locals_dict, &circle_collision_shape_2d_class_locals_dict
);