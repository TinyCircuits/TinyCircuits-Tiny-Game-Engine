#include "vector2.h"
#include "debug/debug_print.h"

// Class required functions
STATIC void vector2_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    vector2_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): Vector2 [%0.3f, %0.3f]", (double)self->x, (double)self->y);
}


mp_obj_t vector2_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Vector2");
    vector2_class_obj_t *self = m_new_obj(vector2_class_obj_t);

    self->base.type = &vector2_class_type;

    if(n_args == 0){
        self->base.type = &vector2_class_type;
        self->x = 0.0f;
        self->y = 0.0f;
    }else if(n_args == 2){
        self->x = mp_obj_get_float(args[0]);
        self->y = mp_obj_get_float(args[1]);
    }else{
        mp_raise_TypeError("function takes 0 or 2 arguments");
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t vector2_class_dot(mp_obj_t self_in, mp_obj_t vector_b){
    const vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const vector2_class_obj_t* b = MP_OBJ_TO_PTR(vector_b);
    return mp_obj_new_float(self->x*b->x + self->y*b->y);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector2_class_dot_obj, vector2_class_dot);


STATIC mp_obj_t vector2_class_cross(mp_obj_t self_in, mp_obj_t vector_b){
    const vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const vector2_class_obj_t* b = MP_OBJ_TO_PTR(vector_b);
    return mp_obj_new_float(self->x*b->y - self->y*b->x);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector2_class_cross_obj, vector2_class_cross);


STATIC mp_obj_t vector2_class_len2(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector2_class_type)){
        mp_raise_TypeError("expected vector argument");
    }
    const vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(self->x*self->x + self->y*self->y);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector2_class_len2_obj, vector2_class_len2);


STATIC mp_obj_t vector2_class_len(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector2_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    const vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_float(sqrt(self->x*self->x + self->y*self->y));
}
MP_DEFINE_CONST_FUN_OBJ_1(vector2_class_len_obj, vector2_class_len);


STATIC mp_obj_t vector2_class_normal(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector2_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    const vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    vector2_class_obj_t* ret = m_new_obj(vector2_class_obj_t);
    ret->base.type = &vector2_class_type;
    const mp_float_t il = 1.0 / sqrt(self->x*self->x + self->y*self->y);
    ret->x = self->x * il;
    ret->y = self->y * il;
    return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector2_class_normal_obj, vector2_class_normal);


STATIC mp_obj_t vector2_class_normalize(mp_obj_t self_in){
    if(!mp_obj_is_type(self_in, &vector2_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    const mp_float_t il = 1.0 / sqrt(self->x*self->x + self->y*self->y);
    self->x = self->x * il;
    self->y = self->y * il;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector2_class_normalize_obj, vector2_class_normalize);


STATIC mp_obj_t vector2_class_rotateZ(mp_obj_t self_in, mp_obj_t _theta){
    if(!mp_obj_is_type(self_in, &vector2_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_float_t xp, yp;

    if(mp_obj_is_type(_theta, &vector2_class_type)){ // Rotate by vector theta (sin(t), cos(t), ~)
        const vector2_class_obj_t* b = MP_OBJ_TO_PTR(_theta);
        xp = self->x*b->y - self->y*b->x;
        yp = self->y*b->y + self->x*b->x;
    }else if(mp_obj_is_float(_theta)){ // Rotate by scalar theta
        const mp_float_t b = mp_obj_get_float(_theta);
        const mp_float_t s = sin(b);
        const mp_float_t c = cos(b);
        xp = self->x*c - self->y*s;
        yp = self->y*c + self->x*s;
    }else{
        mp_raise_TypeError("expected vector or scalar length");
    }

    self->x = xp;
    self->y = yp;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector2_class_rotateZ_obj, vector2_class_rotateZ);


// Set vector to be the same size as another vector or length
STATIC mp_obj_t vector2_class_resize(mp_obj_t self_in, mp_obj_t element_b){
    if(!mp_obj_is_type(self_in, &vector2_class_type)){
        mp_raise_TypeError("expected vector argument");
    }

    vector2_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_float_t f;

    if(mp_obj_is_type(element_b, &vector2_class_type)){ // Resize to match vector length
        const vector2_class_obj_t* b = MP_OBJ_TO_PTR(element_b);
        f = sqrt((b->x*b->x + b->y*b->y) / (self->x*self->x + self->y*self->y));
    }else if(mp_obj_is_float(element_b)){ // Resize to match scalar length
        const mp_float_t b = mp_obj_get_float(element_b);
        f = sqrt((b*b) / (self->x*self->x + self->y*self->y));
    }else{
        mp_raise_TypeError("expected vector or scalar length");
    }

    self->x = self->x*f;
    self->y = self->y*f;
    return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector2_class_resize_obj, vector2_class_resize);


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void vector2_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Vector2 attr");

    vector2_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_x:
                destination[0] = mp_obj_new_float(self->x);
            break;
            case MP_QSTR_y:
                destination[0] = mp_obj_new_float(self->y);
            break;
            case MP_QSTR_dot:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_dot_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_cross:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_cross_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_len2:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_len2_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_len:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_len_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_normal:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_normal_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_normalize:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_normalize_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_resize:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_resize_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_rotateZ:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_rotateZ_obj);
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
        default:
            return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t vector2_class_locals_dict_table[] = {

};


// Class init
STATIC MP_DEFINE_CONST_DICT(vector2_class_locals_dict, vector2_class_locals_dict_table);

const mp_obj_type_t vector2_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Vector2,
    .print = vector2_class_print,
    .make_new = vector2_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = vector2_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&vector2_class_locals_dict,
};