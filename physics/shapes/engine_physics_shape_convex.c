#include "engine_physics_shape_convex.h"
#include "debug/debug_print.h"
#include "physics/engine_physics.h"


// Class required functions
STATIC void physics_shape_convex_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    physics_shape_convex_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): PhysicsShapeConvex");
}


mp_obj_t physics_shape_convex_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New PhysicsShapeConvex");
    physics_shape_convex_class_obj_t *self = m_new_obj(physics_shape_convex_class_obj_t);

    self->base.type = &physics_shape_convex_class_type;

    // if(n_args == 0){
    //     self->base.type = &physics_shape_convex_class_type;
    //     self->radius = 5.0f;
    // }else if(n_args == 1){
    //     self->base.type = &physics_shape_convex_class_type;
    //     self->radius = mp_obj_get_float(args[0]);
    // }else{
    //     mp_raise_TypeError("PhysicsShapeCircle Error: Function takes 0 or 1 arguments");
    // }

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC void physics_shape_convex_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PhysicsShapeConvex attr");

    physics_shape_convex_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_v_list:
                destination[0] = self->v_list;
            break;
            case MP_QSTR_n_list:
                destination[0] = self->n_list;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_v_list:
                self->v_list = destination[1];
            break;
            case MP_QSTR_n_list:
                self->n_list = destination[1];
            break;
        default:
            return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t physics_shape_convex_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(physics_shape_convex_class_locals_dict, physics_shape_convex_class_locals_dict_table);

const mp_obj_type_t physics_shape_convex_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_PhysicsShapeConvex,
    .print = physics_shape_convex_class_print,
    .make_new = physics_shape_convex_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = physics_shape_convex_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&physics_shape_convex_class_locals_dict,
};
