#include "engine_physics_manifold.h"
#include "debug/debug_print.h"
#include "physics/engine_physics.h"


// Class required functions
STATIC void physics_manifold_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    physics_manifold_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): PhysicsManifold");
}

mp_obj_t physics_manifold_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New PhysicsManifold");
    physics_manifold_class_obj_t *self = m_new_obj(physics_manifold_class_obj_t);

    self->base.type = &physics_manifold_class_type;

    // if(n_args == 0){
    //     self->base.type = &physics_shape_circle_class_type;
    //     self->radius = 5.0f;
    // }else if(n_args == 1){
    //     self->base.type = &physics_shape_circle_class_type;
    //     self->radius = mp_obj_get_float(args[0]);
    // }else{
    //     mp_raise_TypeError("PhysicsShapeCircle Error: Function takes 0 or 1 arguments");
    // }

    return MP_OBJ_FROM_PTR(self);
}

// Class methods
STATIC void physics_manifold_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Physicsmanifold attr");

    physics_manifold_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_mtv_x:
                destination[0] = mp_obj_new_float(self->mtv_x);
            break;
            case MP_QSTR_mtv_y:
                destination[0] = mp_obj_new_float(self->mtv_y);
            break;
            case MP_QSTR_nrm_x:
                destination[0] = mp_obj_new_float(self->nrm_x);
            break;
            case MP_QSTR_nrm_y:
                destination[0] = mp_obj_new_float(self->nrm_y);
            break;
            case MP_QSTR_con_x:
                destination[0] = mp_obj_new_float(self->con_x);
            break;
            case MP_QSTR_con_y:
                destination[0] = mp_obj_new_float(self->con_y);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_mtv_x:
                self->mtv_x = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_mtv_y:
                self->mtv_y = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_nrm_x:
                self->nrm_x = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_nrm_y:
                self->nrm_y = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_con_x:
                self->con_x = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_con_y:
                self->con_y = mp_obj_get_float(destination[1]);
            break;
        default:
            return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t physics_manifold_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(physics_manifold_class_locals_dict, physics_manifold_class_locals_dict_table);

const mp_obj_type_t physics_manifold_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_PhysicsManifold,
    .print = physics_manifold_class_print,
    .make_new = physics_manifold_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = physics_manifold_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&physics_manifold_class_locals_dict,
};
