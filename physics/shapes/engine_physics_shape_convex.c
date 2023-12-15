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
    self->I = 0.0;
    self->area = 0.0;
    self->center = m_new_obj(vector2_class_obj_t);
    ((vector2_class_obj_t*)self->center)->base.type = &vector2_class_type;

    if(n_args == 0){
        self->v_list = mp_obj_new_list(0, NULL);
        self->n_list = mp_obj_new_list(0, NULL);
    }else if(n_args == 1){
        if(mp_obj_is_type(args[0], &mp_type_list)) {
            self->v_list = args[0];
            self->base.type = &physics_shape_convex_class_type;
            vector2_class_obj_t t = {{&vector2_class_type}, (mp_float_t)(0.0), (mp_float_t)0.0};
            vector2_class_obj_t r = {{&vector2_class_type}, (mp_float_t)(0.0), (mp_float_t)1.0};
            physics_shape_convex_class_compute_normals(MP_OBJ_FROM_PTR(self));
            physics_shape_convex_class_compute_all(MP_OBJ_FROM_PTR(self));
            physics_shape_convex_class_compute_transform(MP_OBJ_FROM_PTR(self), &t, &r);
        } else {
            mp_raise_TypeError("Expected vertex list argument");
        }
    }else{
        mp_raise_TypeError("PhysicsShapeConvex Error: Function takes 0 or 1 arguments");
    }

    return MP_OBJ_FROM_PTR(self);
}

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

STATIC mp_obj_t physics_shape_convex_class_compute_all(mp_obj_t self_in){

    physics_shape_convex_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t* vs;
    size_t vs_len;
    mp_obj_list_get(self->v_list, &vs_len, &vs);

    vector2_class_obj_t* center = MP_OBJ_TO_PTR(self->center);

    center->x = 0.0;
    center->y = 0.0;
    mp_float_t area = 0.0;
    mp_float_t I = 0.0;

    const mp_float_t inv3 = MICROPY_FLOAT_CONST(1.0/3.0);

    vector2_class_obj_t* s = MP_OBJ_TO_PTR(vs[0]);

    for(int i = 0; i < vs_len; ++i) {
        vector2_class_obj_t* vi = MP_OBJ_TO_PTR(vs[i]);
        vector2_class_obj_t* vi2;

        mp_float_t e1x = vi->x - s->x;
        mp_float_t e1y = vi->y - s->y;

        if(i + 1 < vs_len) vi2 = MP_OBJ_TO_PTR(vs[i+1]);
        else vi2 = MP_OBJ_TO_PTR(vs[0]);

        mp_float_t e2x = vi2->x - s->x;
        mp_float_t e2y = vi2->y - s->y;

        mp_float_t D = e1x*e2y - e2x*e1y;
        mp_float_t t_area = D * 0.5;

        area += t_area;

        center->x += t_area * inv3 * (e1x + e2x);
        center->y += t_area * inv3 * (e1y + e2y);

        mp_float_t ix2 = e1x*e1x + e2x*e1x + e2x*e2x;
        mp_float_t iy2 = e1y*e1y + e2y*e1y + e2y*e2y;

        I += (0.25 * inv3 * D) * (ix2 + iy2);
    }

    self->area = area;
    self->I = I;

    mp_float_t inv_area = 1.0/area;

    center->x *= inv_area;
    center->y *= inv_area;

    center->x += s->x;
    center->y += s->y;

    // self->I += area * (center->x * center->x - (center->x - s->x) * (center->x - s->x) + center->y * center->y - (center->y - s->y) * (center->y - s->y));
    self->I += area * (s->x*(2*center->x - s->x) + s->y*(2*center->y - s->y));

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(physics_shape_convex_class_compute_all_obj, physics_shape_convex_class_compute_all);

STATIC mp_obj_t physics_shape_convex_class_compute_transform(mp_obj_t self_in, mp_obj_t translate_in, mp_obj_t rot_in){

    physics_shape_convex_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t* vs;
    size_t vs_len;
    mp_obj_list_get(self->v_list, &vs_len, &vs);

    mp_obj_t* ns;
    size_t ns_len;
    mp_obj_list_get(self->n_list, &ns_len, &ns);

    vector2_class_obj_t* translate = MP_OBJ_TO_PTR(translate_in);
    vector2_class_obj_t* rot = MP_OBJ_TO_PTR(rot_in);

    mp_obj_t vs_t[vs_len];
    mp_obj_t ns_t[vs_len];

    for(int i = 0; i < vs_len; i++) {
        vector2_class_obj_t* v = MP_OBJ_TO_PTR(vs[i]);
        vector2_class_obj_t* n = MP_OBJ_TO_PTR(ns[i]);

        vs_t[i] = m_new_obj(vector2_class_obj_t);
        vector2_class_obj_t* v_t = MP_OBJ_TO_PTR(vs_t[i]);
        v_t->base.type = &vector2_class_type;
        v_t->x = (v->x * rot->y - v->y * rot->x) + translate->x;
        v_t->y = (v->y * rot->y + v->x * rot->x) + translate->y;

        ns_t[i] = m_new_obj(vector2_class_obj_t);
        vector2_class_obj_t* n_t = MP_OBJ_TO_PTR(vs_t[i]);
        n_t->base.type = &vector2_class_type;
        n_t->x = (n->x * rot->y - n->y * rot->x);
        n_t->y = (n->y * rot->y + n->x * rot->x);
    }

    self->v_t_list = mp_obj_new_list(vs_len, vs_t);
    self->n_t_list = mp_obj_new_list(vs_len, ns_t);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(physics_shape_convex_class_compute_transform_obj, physics_shape_convex_class_compute_transform);


// Class methods
STATIC void physics_shape_convex_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing PhysicsShapeConvex attr");

    physics_shape_convex_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_v_list:
                destination[0] = self->v_list;
            break;
            case MP_QSTR_I:
                destination[0] = mp_obj_new_float(self->I);
            break;
            case MP_QSTR_area:
                destination[0] = mp_obj_new_float(self->area);
            break;
            case MP_QSTR_center:
                destination[0] = self->center;
            break;
            case MP_QSTR_n_list:
                destination[0] = self->n_list;
            break;
            case MP_QSTR_compute_normals:
                destination[0] = MP_OBJ_FROM_PTR(&physics_shape_convex_class_compute_normals_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_compute_all:
                destination[0] = MP_OBJ_FROM_PTR(&physics_shape_convex_class_compute_all_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_compute_transform:
                destination[0] = MP_OBJ_FROM_PTR(&physics_shape_convex_class_compute_transform_obj);
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
            case MP_QSTR_I:
                self->I = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_area:
                self->area = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_center:
                self->center = destination[1];
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
