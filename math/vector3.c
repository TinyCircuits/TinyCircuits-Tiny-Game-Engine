#include "vector3.h"
#include "utility/debug_print.h"

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
    } else {
      mp_raise_TypeError("function takes 0 or 3 arguments");
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t vector3_class_dot(size_t n_args, const mp_obj_t *args) {
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(args[0]);
  const vector3_class_obj_t* b = MP_OBJ_TO_PTR(args[1]);
  return mp_obj_new_float(self->x*b->x + self->y*b->y + self->z*b->z);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(vector3_class_dot_obj, 2, 2, vector3_class_dot);

STATIC mp_obj_t vector3_class_cross(size_t n_args, const mp_obj_t *args) {
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(args[0]);
  const vector3_class_obj_t* b = MP_OBJ_TO_PTR(args[1]);
  vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
  ret->base.type = &vector3_class_type;
  ret->x = self->y*b->z - self->z*b->y;
  ret->y = self->z*b->x - self->x*b->z;
  ret->z = self->x*b->y - self->y*b->x;
  return MP_OBJ_FROM_PTR(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(vector3_class_cross_obj, 2, 2, vector3_class_cross);

// STATIC mp_obj_t vector3_class_dot(size_t n_args, const mp_obj_t *args) {
//     const vector3_class_obj_t* self = MP_OBJ_TO_PTR(args[0]);
//     const vector3_class_obj_t* b = MP_OBJ_TO_PTR(args[1]);
//     return mp_obj_new_float(self->x*b->x + self->y*b->y);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(vector3_class_dot_obj, 2, 2, vector3_class_dot);

// STATIC mp_obj_t vector2_class_cross(mp_obj_t _self, mp_obj_t _b) {
//   const vector3_class_obj_t* self = MP_OBJ_TO_PTR(self);
//   const vector3_class_obj_t* b = MP_OBJ_TO_PTR(b);
//
//   vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);

//   return MP_OBJ_FROM_PTR(ret);
// }


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void vector3_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    vector3_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_x:
                destination[0] = (mp_obj_t*)(&self->x);
            break;
            case MP_QSTR_y:
                destination[0] = (mp_obj_t*)(&self->y);
            break;
            case MP_QSTR_z:
                destination[0] = (mp_obj_t*)(&self->z);
            break;
            case MP_QSTR_dot:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_dot_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_cross:
                destination[0] = MP_OBJ_FROM_PTR(&vector3_class_cross_obj);
                destination[1] = self_in;
            break;
            default:
            break;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        if(attribute == MP_QSTR_x){
            self->x = mp_obj_get_float(destination[1]);
        }else if(attribute == MP_QSTR_y){
            self->y = mp_obj_get_float(destination[1]);
        }else if(attribute == MP_QSTR_z){
            self->z = mp_obj_get_float(destination[1]);
        }else{
            return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t vector3_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_dot), MP_ROM_PTR(&vector3_class_dot_obj) },
    { MP_ROM_QSTR(MP_QSTR_cross), MP_ROM_PTR(&vector3_class_cross_obj) },
};


// Class init
STATIC MP_DEFINE_CONST_DICT(vector3_class_locals_dict, vector3_class_locals_dict_table);

const mp_obj_type_t vector3_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Vector3,
    .print = vector3_class_print,
    .make_new = vector3_class_new,
    // .call = NULL,
    // .unary_op = NULL,
    // .binary_op = NULL,
    .attr = vector3_class_attr,
    // .subscr = NULL,
    // .getiter = NULL,
    // .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&vector3_class_locals_dict,
};
