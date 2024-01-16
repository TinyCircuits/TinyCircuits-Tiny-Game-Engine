#include "rectangle_collision_shape_2d.h"


// Class required functions
STATIC void rectangle_collision_shape_2d_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): RectangleCollisionShape2D");
}


mp_obj_t rectangle_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New RectangleCollisionShape2D");
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    rectangle_collision_shape_2d_class_obj_t *self = m_new_obj(rectangle_collision_shape_2d_class_obj_t);
    self->base.type = &rectangle_collision_shape_2d_class_type;
    self->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    self->width = mp_obj_get_float(args[0]);
    self->height = mp_obj_get_float(args[1]);
    
    return MP_OBJ_FROM_PTR(self);
}


STATIC void rectangle_collision_shape_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing RectangleCollisionShape2D attr");

    rectangle_collision_shape_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_width:
                destination[0] = mp_obj_new_float(self->width);
            break;
            case MP_QSTR_height:
                destination[0] = mp_obj_new_float(self->height);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_width:
                self->width = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_height:
                self->height = mp_obj_get_float(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_collision_shape_2d_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(rectangle_collision_shape_2d_class_locals_dict, rectangle_collision_shape_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    rectangle_collision_shape_2d_class_type,
    MP_QSTR_RectangleCollisionShape2D,
    MP_TYPE_FLAG_NONE,

    make_new, rectangle_collision_shape_2d_class_new,
    print, rectangle_collision_shape_2d_class_print,
    attr, rectangle_collision_shape_2d_class_attr,
    locals_dict, &rectangle_collision_shape_2d_class_locals_dict
);