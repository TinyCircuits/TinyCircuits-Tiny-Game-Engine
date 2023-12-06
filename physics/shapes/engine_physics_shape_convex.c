#include "engine_physics_shape_convex.h"
#include "debug/debug_print.h"
#include "physics/engine_physics.h"
#include "math/vector2.h"


// Class required functions
STATIC void physics_shape_convex_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    physics_shape_convex_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): PhysicsShapeConvex");
}


mp_obj_t physics_shape_convex_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New PhysicsShapeConvex");
    physics_shape_convex_class_obj_t *self = m_new_obj(physics_shape_convex_class_obj_t);

    self->base.type = &physics_shape_convex_class_type;

    if(n_args == 0){
        self->v_list = mp_obj_new_list(0, NULL);
        self->n_list = mp_obj_new_list(0, NULL);
    }else if(n_args == 1){
        if(mp_obj_is_type(args[0], &mp_type_list)) {
            self->v_list = args[0];
            self->base.type = &physics_shape_convex_class_type;
        } else {
            mp_raise_TypeError("Expected vertex list argument");
        }
    }else{
        mp_raise_TypeError("PhysicsShapeConvex Error: Function takes 0 or 1 arguments");
    }

    return MP_OBJ_FROM_PTR(self);
}

// Set vector to be the same size as another vector or length
STATIC mp_obj_t physics_shape_convex_class_compute_normals(mp_obj_t self_in){

    physics_shape_convex_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t* vs;
    size_t vs_len;
    mp_obj_list_get(self->v_list, &vs_len, &vs);

    mp_obj_t ns[vs_len];
    size_t ns_len = vs_len;

    if(vs_len > 1) for(int i = 0; i < vs_len-1; i++) {
        vector2_class_obj_t* v1 = MP_OBJ_TO_PTR(vs[i]);
        vector2_class_obj_t* v2 = MP_OBJ_TO_PTR(vs[i+1]);
        mp_float_t dx = v2->x - v1->x;
        mp_float_t dy = v2->y - v1->y;
        mp_float_t idist = 1.0/MICROPY_FLOAT_C_FUN(sqrt)(dx*dx+dy*dy);
        ns[i] = m_new_obj(vector2_class_obj_t);
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(ns[i]);
        n->base.type = &vector2_class_type;
        n->x = -dy*idist;
        n->y = dx*idist;
    }
    if(vs_len > 1) {
        vector2_class_obj_t* v1 = MP_OBJ_TO_PTR(vs[vs_len-1]);
        vector2_class_obj_t* v2 = MP_OBJ_TO_PTR(vs[0]);
        mp_float_t dx = v2->x - v1->x;
        mp_float_t dy = v2->y - v1->y;
        mp_float_t idist = 1.0/MICROPY_FLOAT_C_FUN(sqrt)(dx*dx+dy*dy);
        ns[ns_len-1] = m_new_obj(vector2_class_obj_t);
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(ns[ns_len-1]);
        n->base.type = &vector2_class_type;
        n->x = -dy*idist;
        n->y = dx*idist;
    }

    self->n_list = mp_obj_new_list(ns_len, ns);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(physics_shape_convex_class_compute_normals_obj, physics_shape_convex_class_compute_normals);


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
            case MP_QSTR_compute_normals:
                destination[0] = MP_OBJ_FROM_PTR(&physics_shape_convex_class_compute_normals_obj);
                destination[1] = self_in;
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
