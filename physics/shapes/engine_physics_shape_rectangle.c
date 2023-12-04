#include "engine_physics_shape_rectangle.h"
#include "debug/debug_print.h"
#include "physics/engine_physics.h"


// Class required functions
STATIC void physics_shape_rectangle_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    physics_shape_rectangle_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): PhysicsShapeRectangle");
}


mp_obj_t physics_shape_rectangle_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New PhysicsShapeRectangle");
    physics_shape_rectangle_class_obj_t *self = m_new_obj(physics_shape_rectangle_class_obj_t);

    self->base.type = &physics_shape_rectangle_class_type;

    if(n_args == 0){
        self->base.type = &physics_shape_rectangle_class_type;
        self->width = 15.0f;
        self->height = 5.0f;
    }else if(n_args == 2){
        self->base.type = &physics_shape_rectangle_class_type;
        self->width = mp_obj_get_float(args[0]);
        self->height = mp_obj_get_float(args[1]);
    }else{
        mp_raise_TypeError("PhysicsShapeRectangle Error: Function takes 0 or 2 arguments");
    }

    return MP_OBJ_FROM_PTR(self);
}

// STATIC mp_obj_t physics_shape_rectangle_class_test(mp_obj_t self_in){
//     if(!mp_obj_is_type(self_in, &physics_shape_rectangle_class_type)){
//         mp_raise_TypeError("expected rectangle object");
//     }
//
//     const physics_shape_rectangle_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
//     vector2_class_obj_t* ret = m_new_obj(vector2_class_obj_t);
//     ret->base.type = &vector2_class_type;
//     ret->x = 0.0;
//     ret->y = 0.0;
//     return MP_OBJ_FROM_PTR(ret);
// }
// MP_DEFINE_CONST_FUN_OBJ_1(physics_shape_rectangle_class_test_obj, physics_shape_rectangle_class_test);

// Class methods
STATIC void physics_shape_rectangle_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PhysicsShapeRectangle attr");

    physics_shape_rectangle_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_width:
                destination[0] = mp_obj_new_float(self->width);
            break;
            case MP_QSTR_height:
                destination[0] = mp_obj_new_float(self->height);
            break;
            // case MP_QSTR_test:
            //     destination[0] = MP_OBJ_FROM_PTR(&physics_shape_rectangle_class_test_obj);
            //     destination[1] = self_in;
            // break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
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
STATIC const mp_rom_map_elem_t physics_shape_rectangle_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(physics_shape_rectangle_class_locals_dict, physics_shape_rectangle_class_locals_dict_table);

const mp_obj_type_t physics_shape_rectangle_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_PhysicsShapeRectangle,
    .print = physics_shape_rectangle_class_print,
    .make_new = physics_shape_rectangle_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = physics_shape_rectangle_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&physics_shape_rectangle_class_locals_dict,
};
