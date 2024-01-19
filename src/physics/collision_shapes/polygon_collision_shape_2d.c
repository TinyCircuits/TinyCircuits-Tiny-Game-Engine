#include "polygon_collision_shape_2d.h"


// Class required functions
STATIC void polygon_collision_shape_2d_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): PolygonCollisionShape2D");
}


mp_obj_t polygon_collision_shape_2d_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New PolygonCollisionShape2D");
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    polygon_collision_shape_2d_class_obj_t *self = m_new_obj(polygon_collision_shape_2d_class_obj_t);
    self->base.type = &polygon_collision_shape_2d_class_type;
    self->position = vector2_class_new(&vector2_class_type, 0, 0, NULL);
    self->vertices = mp_obj_new_list(0, NULL);
    
    return MP_OBJ_FROM_PTR(self);
}


STATIC void polygon_collision_shape_2d_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PolygonCollisionShape2D attr");

    polygon_collision_shape_2d_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_position:
                destination[0] = self->position;
            break;
            case MP_QSTR_vertices:
                destination[0] = self->vertices;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_position:
                self->position = destination[1];
            break;
            case MP_QSTR_vertices:
                self->vertices = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t polygon_collision_shape_2d_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(polygon_collision_shape_2d_class_locals_dict, polygon_collision_shape_2d_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    polygon_collision_shape_2d_class_type,
    MP_QSTR_PolygonCollisionShape2D,
    MP_TYPE_FLAG_NONE,

    make_new, polygon_collision_shape_2d_class_new,
    print, polygon_collision_shape_2d_class_print,
    attr, polygon_collision_shape_2d_class_attr,
    locals_dict, &polygon_collision_shape_2d_class_locals_dict
);