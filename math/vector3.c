#include "vector3.h"
#include "debug/debug_print.h"

// Class required functions
STATIC void vector3_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    vector3_class_obj_t *self = self_in;
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    ENGINE_INFO_PRINTF("print(): Vector3 [%0.3f, %0.3f, %0.3f]", (double)x, (double)y, (double)z);
}


mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Vector3");
    vector3_class_obj_t *self = m_new_obj(vector3_class_obj_t);

    if(n_args == 0) {
      self->base.type = &vector3_class_type;
      self->x = mp_obj_new_float(0.0);
      self->y = mp_obj_new_float(0.0);
      self->z = mp_obj_new_float(0.0);
    } else if(n_args == 3) {
      self->base.type = &vector3_class_type;
      self->x = args[0];
      self->y = args[1];
      self->z = args[2];
    }else{
      mp_raise_TypeError("function takes 0 or 3 arguments");
    }
    ENGINE_INFO_PRINTF("Returning Vector3");
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t vector3_class_dot(mp_obj_t self_in, mp_obj_t vector_b){
    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(vector_b);
    const mp_float_t ax = mp_obj_get_float(self->x);
    const mp_float_t ay = mp_obj_get_float(self->y);
    const mp_float_t az = mp_obj_get_float(self->z);
    const mp_float_t bx = mp_obj_get_float(b->x);
    const mp_float_t by = mp_obj_get_float(b->y);
    const mp_float_t bz = mp_obj_get_float(b->z);
    return mp_obj_new_float(ax*bx + ay*by + az*bz);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_dot_obj, vector3_class_dot);


STATIC mp_obj_t vector3_class_cross(mp_obj_t self_in, mp_obj_t _b){
    if(!mp_obj_is_type(self_in, &vector3_class_type) || !mp_obj_is_type(_b, &vector3_class_type)){
        mp_raise_TypeError("expected vector arguments");
    }

    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_b);
    vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
    ret->base.type = &vector3_class_type;
    const mp_float_t ax = mp_obj_get_float(self->x);
    const mp_float_t ay = mp_obj_get_float(self->y);
    const mp_float_t az = mp_obj_get_float(self->z);
    const mp_float_t bx = mp_obj_get_float(b->x);
    const mp_float_t by = mp_obj_get_float(b->y);
    const mp_float_t bz = mp_obj_get_float(b->z);
    ret->x = mp_obj_new_float(ay*bz - az*by);
    ret->y = mp_obj_new_float(az*bx - ax*bz);
    ret->z = mp_obj_new_float(ax*by - ay*bx);
    return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_cross_obj, vector3_class_cross);


STATIC mp_obj_t vector3_class_len_squared(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
    }
    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    return mp_obj_new_float(x*x + y*y + z*z);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_len_squared_obj, vector3_class_len_squared);


STATIC mp_obj_t vector3_class_len(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    return mp_obj_new_float(sqrt(x*x + y*y + z*z));
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_len_obj, vector3_class_len);


STATIC mp_obj_t vector3_class_normal(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
    ret->base.type = &vector3_class_type;
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    const mp_float_t il = 1.0 / sqrt(x*x + y*y + z*z);
    ret->x = mp_obj_new_float(x * il);
    ret->y = mp_obj_new_float(y * il);
    ret->z = mp_obj_new_float(z * il);
    return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_normal_obj, vector3_class_normal);


STATIC mp_obj_t vector3_class_normalize(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    const mp_float_t il = 1.0 / sqrt(x*x + y*y + z*z);
    self->x = mp_obj_new_float(x * il);
    self->y = mp_obj_new_float(y * il);
    self->z = mp_obj_new_float(z * il);
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_normalize_obj, vector3_class_normalize);


STATIC mp_obj_t vector3_class_rotateZ(mp_obj_t self_in, mp_obj_t _theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_float_t xp, yp;
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);

    if(mp_obj_is_type(_theta, &vector3_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_theta);
        const mp_float_t bx = mp_obj_get_float(b->x);
        const mp_float_t by = mp_obj_get_float(b->y);
        xp = x*by - y*bx;
        yp = y*by + x*bx;
    }else if(mp_obj_is_float(_theta)){ // Rotate by scalar theta
        const mp_float_t b = mp_obj_get_float(_theta);
        const mp_float_t s = sin(b);
        const mp_float_t c = cos(b);
        xp = x*c - y*s;
        yp = y*c + x*s;
    }else{
        mp_raise_TypeError("expected vector or scalar length");
    }

    self->x = mp_obj_new_float(xp);
    self->y = mp_obj_new_float(yp);
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateZ_obj, vector3_class_rotateZ);


STATIC mp_obj_t vector3_class_rotateY(mp_obj_t self_in, mp_obj_t theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_float_t xp, zp;
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t z = mp_obj_get_float(self->z);

    if(mp_obj_is_type(theta, &vector3_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(theta);
        const mp_float_t bx = mp_obj_get_float(b->x);
        const mp_float_t by = mp_obj_get_float(b->y);
        xp = x*by - z*bx;
        zp = z*by + x*bx;
    }else if(mp_obj_is_float(theta)){ // Rotate by scalar theta
        const mp_float_t b = mp_obj_get_float(theta);
        const mp_float_t s = sin(b);
        const mp_float_t c = cos(b);
        xp = x*c - z*s;
        zp = z*c + x*s;
    }else{
        mp_raise_TypeError("expected vector or scalar length");
    }

    self->x = mp_obj_new_float(xp);
    self->z = mp_obj_new_float(zp);
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateY_obj, vector3_class_rotateY);


STATIC mp_obj_t vector3_class_rotateX(mp_obj_t self_in, mp_obj_t theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_float_t yp, zp;
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);

    if(mp_obj_is_type(theta, &vector3_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(theta);
        const mp_float_t bx = mp_obj_get_float(b->x);
        const mp_float_t by = mp_obj_get_float(b->y);
        yp = y*by - z*bx;
        zp = z*by + y*bx;
    }else if(mp_obj_is_float(theta)){ // Rotate by scalar theta
        const mp_float_t b = mp_obj_get_float(theta);
        const mp_float_t s = sin(b);
        const mp_float_t c = cos(b);
        yp = y*c - z*s;
        zp = z*c + y*s;
    }else{
        mp_raise_TypeError("expected vector or scalar length");
    }

    self->y = mp_obj_new_float(yp);
    self->z = mp_obj_new_float(zp);
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateX_obj, vector3_class_rotateX);


static void q_mul(const mp_float_t* q1, const mp_float_t* q2, mp_float_t* y){
    y[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    y[1] = q1[0]*q2[1] + q1[1]*q2[0] - q1[2]*q2[3] + q1[3]*q2[2];
    y[2] = q1[0]*q2[2] + q1[1]*q2[3] + q1[2]*q2[0] - q1[3]*q2[1];
    y[3] = q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1] + q1[3]*q2[0];
}


static void q_rot_mul(const mp_float_t* q1, const mp_float_t* q2, mp_float_t* y){ // Quaternion multiply assuming real component of q2 is zero
    y[0] =  - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    y[1] =    q1[0]*q2[1] - q1[2]*q2[3] + q1[3]*q2[2];
    y[2] =    q1[0]*q2[2] + q1[1]*q2[3] - q1[3]*q2[1];
    y[3] =    q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1];
}


// Rotate by axis-angle form
STATIC mp_obj_t vector3_class_rotate(mp_obj_t self_in, mp_obj_t about_axis, mp_obj_t theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }
    if(!mp_obj_is_type(about_axis, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }
    if(!mp_obj_is_float(theta)){
        mp_raise_TypeError("expected scalar angle");
    }
    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    vector3_class_obj_t* axis = MP_OBJ_TO_PTR(about_axis);
    const mp_float_t ax = mp_obj_get_float(axis->x);
    const mp_float_t ay = mp_obj_get_float(axis->y);


    const mp_float_t az = mp_obj_get_float(axis->z);

    const mp_float_t s = sin(mp_obj_get_float(theta)*0.5);
    const mp_float_t qa[4] = {cos(mp_obj_get_float(theta)*0.5), ax * s, ay * s, az * s};
    const mp_float_t qia[4] = {qa[0], -qa[1], -qa[2], -qa[3]};
    const mp_float_t qp[4] = {0.f, x, y, z};

    mp_float_t rot1[4];
    mp_float_t rot2[4];

    // p' = q^(-1) * p * q
    q_rot_mul(qia, qp, rot1);
    q_mul(rot1, qa, rot2);

    self->x = mp_obj_new_float(rot2[1]);
    self->y = mp_obj_new_float(rot2[2]);
    self->z = mp_obj_new_float(rot2[3]);

    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_3(vector3_class_rotate_obj, vector3_class_rotate);


// Set vector to be the same size as another vector or length
STATIC mp_obj_t vector3_class_resize(mp_obj_t self_in, mp_obj_t element_b){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const mp_float_t x = mp_obj_get_float(self->x);
    const mp_float_t y = mp_obj_get_float(self->y);
    const mp_float_t z = mp_obj_get_float(self->z);
    mp_float_t f;

    if(mp_obj_is_type(element_b, &vector3_class_type)){ // Resize to match vector length
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(element_b);
        const mp_float_t bx = mp_obj_get_float(b->x);
        const mp_float_t by = mp_obj_get_float(b->y);
        const mp_float_t bz = mp_obj_get_float(b->z);
        f = sqrt((bx*bx + by*by + bz*bz) / (x*x + y*y + z*z));
    }else if(mp_obj_is_float(element_b)){ // Resize to match scalar length
        const mp_float_t b = mp_obj_get_float(element_b);
        f = sqrt((b*b) / (x*x + y*y + z*z));
    }else{
        mp_raise_TypeError("expected vector or scalar length");
    }

    self->x = mp_obj_new_float(x*f);
    self->y = mp_obj_new_float(y*f);
    self->z = mp_obj_new_float(z*f);
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_resize_obj, vector3_class_resize);


STATIC void vector3_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Vector3 attr");

    vector3_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_x:
                destination[0] = self->x;
            break;
            case MP_QSTR_y:
                destination[0] = self->y;
            break;
            case MP_QSTR_z:
                destination[0] = self->z;
            break;
            case MP_QSTR_dot:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_dot_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_cross:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_cross_obj); 
                destination[1] = self_in;
            break;
            case MP_QSTR_len_squared:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_len_squared_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_len:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_len_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_normal:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_normal_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_normalize:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_normalize_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_resize:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_resize_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_rotateZ:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotateZ_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_rotateY:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotateY_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_rotateX:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotateX_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_rotate:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotate_obj);
                destination[1] = self_in;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_x:
                self->x = destination[1];
            break;
            case MP_QSTR_y:
                self->y = destination[1];
            break;
            case MP_QSTR_z:
                self->z = destination[1];
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


// Class init
STATIC MP_DEFINE_CONST_DICT(vector3_class_locals_dict, vector3_class_locals_dict_table);

const mp_obj_type_t vector3_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Vector3,
    .print = vector3_class_print,
    .make_new = vector3_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = vector3_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&vector3_class_locals_dict,
};