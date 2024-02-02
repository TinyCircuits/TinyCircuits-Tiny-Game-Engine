#include "vector3.h"
#include "debug/debug_print.h"

// Class required functions
STATIC void vector3_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    vector3_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): Vector3 [%0.3f, %0.3f, %0.3f]", (double)self->x, (double)self->y, (double)self->z);
}


mp_obj_t vector3_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Vector3");
    vector3_class_obj_t *self = m_new_obj(vector3_class_obj_t);

    if(n_args == 0) {
      self->base.type = &vector3_class_type;
      self->x = 0.0f;
      self->y = 0.0f;
      self->z = 0.0f;
    } else if(n_args == 3) {
      self->base.type = &vector3_class_type;
      self->x = mp_obj_get_float(args[0]);
      self->y = mp_obj_get_float(args[1]);
      self->z = mp_obj_get_float(args[2]);
    }else{
      mp_raise_TypeError(MP_ERROR_TEXT("function takes 0 or 3 arguments"));
    }
    ENGINE_INFO_PRINTF("Returning Vector3");
    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t vector3_class_dot(mp_obj_t self_in, mp_obj_t vector_b){
    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(vector_b);
    const float ax = self->x;
    const float ay = self->y;
    const float az = self->z;
    const float bx = b->x;
    const float by = b->y;
    const float bz = b->z;
    return mp_obj_new_float(ax*bx + ay*by + az*bz);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_dot_obj, vector3_class_dot);


STATIC mp_obj_t vector3_class_cross(mp_obj_t self_in, mp_obj_t _b){
    if(!mp_obj_is_type(self_in, &vector3_class_type) || !mp_obj_is_type(_b, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector arguments"));
    }

    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_b);
    vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
    ret->base.type = &vector3_class_type;
    const float ax = self->x;
    const float ay = self->y;
    const float az = self->z;
    const float bx = b->x;
    const float by = b->y;
    const float bz = b->z;
    ret->x = ay*bz - az*by;
    ret->y = az*bx - ax*bz;
    ret->z = ax*by - ay*bx;
    return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_cross_obj, vector3_class_cross);


STATIC mp_obj_t vector3_class_len_squared(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }
    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const float x = self->x;
    const float y = self->y;
    const float z = self->z;
    return mp_obj_new_float(x*x + y*y + z*z);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_len_squared_obj, vector3_class_len_squared);


STATIC mp_obj_t vector3_class_len(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const float x = self->x;
    const float y = self->y;
    const float z = self->z;
    return mp_obj_new_float(sqrt(x*x + y*y + z*z));
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_len_obj, vector3_class_len);


STATIC mp_obj_t vector3_class_normal(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
    ret->base.type = &vector3_class_type;
    const float x = self->x;
    const float y = self->y;
    const float z = self->z;
    const float il = 1.0 / sqrt(x*x + y*y + z*z);
    ret->x = x * il;
    ret->y = y * il;
    ret->z = z * il;
    return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_normal_obj, vector3_class_normal);


STATIC mp_obj_t vector3_class_normalize(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const float x = self->x;
    const float y = self->y;
    const float z = self->z;
    const float il = 1.0 / sqrt(x*x + y*y + z*z);
    self->x = x * il;
    self->y = y * il;
    self->z = z * il;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_normalize_obj, vector3_class_normalize);


STATIC mp_obj_t vector3_class_rotateZ(mp_obj_t self_in, mp_obj_t _theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    float xp, yp;
    const float x = self->x;
    const float y = self->y;

    if(mp_obj_is_type(_theta, &vector3_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_theta);
        const float bx = b->x;
        const float by = b->y;
        xp = x*by - y*bx;
        yp = y*by + x*bx;
    }else if(mp_obj_is_float(_theta)){ // Rotate by scalar theta
        const float b = mp_obj_get_float(_theta);
        const float s = sin(b);
        const float c = cos(b);
        xp = x*c - y*s;
        yp = y*c + x*s;
    }else{
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector or scalar length"));
    }

    self->x = xp;
    self->y = yp;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateZ_obj, vector3_class_rotateZ);


STATIC mp_obj_t vector3_class_rotateY(mp_obj_t self_in, mp_obj_t theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    float xp, zp;
    const float x = self->x;
    const float z = self->z;

    if(mp_obj_is_type(theta, &vector3_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(theta);
        const float bx = b->x;
        const float by = b->y;
        xp = x*by - z*bx;
        zp = z*by + x*bx;
    }else if(mp_obj_is_float(theta)){ // Rotate by scalar theta
        const float b = mp_obj_get_float(theta);
        const float s = sin(b);
        const float c = cos(b);
        xp = x*c - z*s;
        zp = z*c + x*s;
    }else{
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector or scalar length"));
    }

    self->x = xp;
    self->z = zp;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateY_obj, vector3_class_rotateY);


STATIC mp_obj_t vector3_class_rotateX(mp_obj_t self_in, mp_obj_t theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    float yp, zp;
    const float y = self->y;
    const float z = self->z;

    if(mp_obj_is_type(theta, &vector3_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(theta);
        const float bx = b->x;
        const float by = b->y;
        yp = y*by - z*bx;
        zp = z*by + y*bx;
    }else if(mp_obj_is_float(theta)){ // Rotate by scalar theta
        const float b = mp_obj_get_float(theta);
        const float s = sin(b);
        const float c = cos(b);
        yp = y*c - z*s;
        zp = z*c + y*s;
    }else{
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector or scalar length"));
    }

    self->y = yp;
    self->z = zp;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateX_obj, vector3_class_rotateX);


static void q_mul(const float* q1, const float* q2, float* y){
    y[0] = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    y[1] = q1[0]*q2[1] + q1[1]*q2[0] - q1[2]*q2[3] + q1[3]*q2[2];
    y[2] = q1[0]*q2[2] + q1[1]*q2[3] + q1[2]*q2[0] - q1[3]*q2[1];
    y[3] = q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1] + q1[3]*q2[0];
}


static void q_rot_mul(const float* q1, const float* q2, float* y){ // Quaternion multiply assuming real component of q2 is zero
    y[0] =  - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
    y[1] =    q1[0]*q2[1] - q1[2]*q2[3] + q1[3]*q2[2];
    y[2] =    q1[0]*q2[2] + q1[1]*q2[3] - q1[3]*q2[1];
    y[3] =    q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1];
}


// Rotate by axis-angle form
STATIC mp_obj_t vector3_class_rotate(mp_obj_t self_in, mp_obj_t about_axis, mp_obj_t theta){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }
    if(!mp_obj_is_type(about_axis, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }
    if(!mp_obj_is_float(theta)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected scalar angle"));
    }
    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const float x = self->x;
    const float y = self->y;
    const float z = self->z;
    vector3_class_obj_t* axis = MP_OBJ_TO_PTR(about_axis);
    const float ax = axis->x;
    const float ay = axis->y;


    const float az = axis->z;

    const float s = sin(mp_obj_get_float(theta)*0.5);
    const float qa[4] = {cos(mp_obj_get_float(theta)*0.5), ax * s, ay * s, az * s};
    const float qia[4] = {qa[0], -qa[1], -qa[2], -qa[3]};
    const float qp[4] = {0.f, x, y, z};

    float rot1[4];
    float rot2[4];

    // p' = q^(-1) * p * q
    q_rot_mul(qia, qp, rot1);
    q_mul(rot1, qa, rot2);

    self->x = rot2[1];
    self->y = rot2[2];
    self->z = rot2[3];

    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_3(vector3_class_rotate_obj, vector3_class_rotate);


// Set vector to be the same size as another vector or length
STATIC mp_obj_t vector3_class_resize(mp_obj_t self_in, mp_obj_t element_b){
    if(!mp_obj_is_type(self_in, &vector3_class_type)){
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector argument"));
    }

    vector3_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const float x = self->x;
    const float y = self->y;
    const float z = self->z;
    float f;

    if(mp_obj_is_type(element_b, &vector3_class_type)){ // Resize to match vector length
        const vector3_class_obj_t* b = MP_OBJ_TO_PTR(element_b);
        const float bx = b->x;
        const float by = b->y;
        const float bz = b->z;
        f = sqrt((bx*bx + by*by + bz*bz) / (x*x + y*y + z*z));
    }else if(mp_obj_is_float(element_b)){ // Resize to match scalar length
        const float b = mp_obj_get_float(element_b);
        f = sqrt((b*b) / (x*x + y*y + z*z));
    }else{
        mp_raise_TypeError(MP_ERROR_TEXT("expected vector or scalar length"));
    }

    self->x = x*f;
    self->y = y*f;
    self->z = z*f;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_resize_obj, vector3_class_resize);


/*  --- doc ---
    NAME: Vector3
    DESC: Holds an X, Y, and Z value. Typically used for position
    PARAM: [type=float]  [name=x]    [value=any]
    PARAM: [type=float]  [name=y]    [value=any]
    PARAM: [type=float]  [name=z]    [value=any]                                                                                      
    ATTR: [type=float]  [name=x]    [value=any]                                
    ATTR: [type=float]  [name=y]    [value=any]
    ATTR: [type=float]  [name=z]    [value=any]                                                               
*/ 
STATIC void vector3_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Vector3 attr");

    vector3_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_x:
                destination[0] = mp_obj_new_float(self->x);
            break;
            case MP_QSTR_y:
                destination[0] = mp_obj_new_float(self->y);
            break;
            case MP_QSTR_z:
                destination[0] = mp_obj_new_float(self->z);
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
                self->x = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_y:
                self->y = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_z:
                self->z = mp_obj_get_float(destination[1]);
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
STATIC MP_DEFINE_CONST_DICT(vector3_class_locals_dict, vector3_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    vector3_class_type,
    MP_QSTR_Vector3,
    MP_TYPE_FLAG_NONE,

    make_new, vector3_class_new,
    print, vector3_class_print,
    attr, vector3_class_attr,
    locals_dict, &vector3_class_locals_dict
);