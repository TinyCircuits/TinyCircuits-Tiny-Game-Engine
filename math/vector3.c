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
STATIC mp_obj_t vector3_class_test(mp_obj_t self){
    ENGINE_INFO_PRINTF("Vector3 test");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_test_obj, vector3_class_test);

STATIC mp_obj_t vector3_class_dot(mp_obj_t _self, mp_obj_t _b) {
  if(!mp_obj_is_type(_self, &vector3_class_type) || !mp_obj_is_type(_b, &vector3_class_type)) {
        mp_raise_TypeError("expected vector arguments");
  }
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_b);
  return mp_obj_new_float(self->x*b->x + self->y*b->y + self->z*b->z);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_dot_obj, vector3_class_dot);

STATIC mp_obj_t vector3_class_cross(mp_obj_t _self, mp_obj_t _b) {
  if(!mp_obj_is_type(_self, &vector3_class_type) || !mp_obj_is_type(_b, &vector3_class_type)) {
        mp_raise_TypeError("expected vector arguments");
  }
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_b);
  vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
  ret->base.type = &vector3_class_type;
  ret->x = self->y*b->z - self->z*b->y;
  ret->y = self->z*b->x - self->x*b->z;
  ret->z = self->x*b->y - self->y*b->x;
  return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_cross_obj, vector3_class_cross);

STATIC mp_obj_t vector3_class_len2(mp_obj_t _self) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  return mp_obj_new_float(self->x*self->x + self->y*self->y + self->z*self->z);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_len2_obj, vector3_class_len2);

STATIC mp_obj_t vector3_class_len(mp_obj_t _self) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  return mp_obj_new_float(sqrtf(self->x*self->x + self->y*self->y + self->z*self->z));
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_len_obj, vector3_class_len);

STATIC mp_obj_t vector3_class_normal(mp_obj_t _self) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  const vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  vector3_class_obj_t* ret = m_new_obj(vector3_class_obj_t);
  ret->base.type = &vector3_class_type;
  const float il = 1.f / sqrtf(self->x*self->x + self->y*self->y + self->z*self->z);
  ret->x = self->x * il;
  ret->y = self->y * il;
  ret->z = self->z * il;
  return MP_OBJ_FROM_PTR(ret);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_normal_obj, vector3_class_normal);

STATIC mp_obj_t vector3_class_normalize(mp_obj_t _self) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  const mp_float_t il = 1.f / sqrtf(self->x*self->x + self->y*self->y + self->z*self->z);
  self->x = self->x * il;
  self->y = self->y * il;
  self->z = self->z * il;
  return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_normalize_obj, vector3_class_normalize);

STATIC mp_obj_t vector3_class_rotateZ(mp_obj_t _self, mp_obj_t _theta) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  mp_float_t xp, yp;
  if(mp_obj_is_type(_theta, &vector3_class_type)) { // Rotate by vector theta (sin(t), cos(t), ~)

    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_theta);
    xp = self->x*b->y - self->y*b->x;
    yp = self->y*b->y + self->x*b->x;

  } else if(mp_obj_is_float(_theta)) { // Rotate by scalar theta

    const float b = mp_obj_get_float(_theta);
    const mp_float_t s = sinf(b);
    const mp_float_t c = cosf(b);
    xp = self->x*c - self->y*s;
    yp = self->y*c + self->x*s;

  } else {
    mp_raise_TypeError("expected vector or scalar length");
  }
  self->x = xp;
  self->y = yp;
  return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateZ_obj, vector3_class_rotateZ);

STATIC mp_obj_t vector3_class_rotateY(mp_obj_t _self, mp_obj_t _theta) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  mp_float_t xp, zp;
  if(mp_obj_is_type(_theta, &vector3_class_type)) { // Rotate by vector theta (sin(t), cos(t), ~)

    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_theta);
    xp = self->x*b->y - self->z*b->x;
    zp = self->z*b->y + self->x*b->x;

  } else if(mp_obj_is_float(_theta)) { // Rotate by scalar theta

    const float b = mp_obj_get_float(_theta);
    const mp_float_t s = sinf(b);
    const mp_float_t c = cosf(b);
    xp = self->x*c - self->z*s;
    zp = self->z*c + self->x*s;

  } else {
    mp_raise_TypeError("expected vector or scalar length");
  }
  self->x = xp;
  self->z = zp;
  return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateY_obj, vector3_class_rotateY);

STATIC mp_obj_t vector3_class_rotateX(mp_obj_t _self, mp_obj_t _theta) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  mp_float_t yp, zp;
  if(mp_obj_is_type(_theta, &vector3_class_type)) { // Rotate by vector theta (sin(t), cos(t), ~)

    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_theta);
    yp = self->y*b->y - self->z*b->x;
    zp = self->z*b->y + self->y*b->x;

  } else if(mp_obj_is_float(_theta)) { // Rotate by scalar theta

    const float b = mp_obj_get_float(_theta);
    const mp_float_t s = sinf(b);
    const mp_float_t c = cosf(b);
    yp = self->y*c - self->z*s;
    zp = self->z*c + self->y*s;

  } else {
    mp_raise_TypeError("expected vector or scalar length");
  }
  self->y = yp;
  self->z = zp;
  return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_2(vector3_class_rotateX_obj, vector3_class_rotateX);

// Set vector to be the same size as another vector or length
STATIC mp_obj_t vector3_class_resize(mp_obj_t _self, mp_obj_t _b) {
  if(!mp_obj_is_type(_self, &vector3_class_type)) {
        mp_raise_TypeError("expected vector argument");
  }
  vector3_class_obj_t* self = MP_OBJ_TO_PTR(_self);
  float f;
  if(mp_obj_is_type(_b, &vector3_class_type)) { // Resize to match vector length

    const vector3_class_obj_t* b = MP_OBJ_TO_PTR(_b);
    f = sqrtf((b->x*b->x + b->y*b->y + b->z*b->z) / (self->x*self->x + self->y*self->y + self->z*self->z));

  } else if(mp_obj_is_float(_b)) { // Resize to match scalar length

    const float b = mp_obj_get_float(_b);
    f = sqrtf((b*b) / (self->x*self->x + self->y*self->y + self->z*self->z));

  } else {
    mp_raise_TypeError("expected vector or scalar length");
  }
  self->x *= f;
  self->y *= f;
  self->z *= f;
  return MP_OBJ_FROM_PTR(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(vector3_class_resize_obj, vector3_class_resize);

// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void vector3_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    vector3_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
          case MP_QSTR_x: destination[0] = (mp_obj_t*)(&self->x); break;
          case MP_QSTR_y: destination[0] = (mp_obj_t*)(&self->y); break;
          case MP_QSTR_z: destination[0] = (mp_obj_t*)(&self->z); break;
          case MP_QSTR_test: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_test_obj); destination[1] = self_in; break;
          case MP_QSTR_dot: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_dot_obj); destination[1] = self_in; break;
          case MP_QSTR_cross: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_cross_obj); destination[1] = self_in; break;
          case MP_QSTR_len2: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_len2_obj); destination[1] = self_in; break;
          case MP_QSTR_len: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_len_obj); destination[1] = self_in; break;
          case MP_QSTR_normal: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_normal_obj); destination[1] = self_in; break;
          case MP_QSTR_normalize: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_normalize_obj); destination[1] = self_in; break;
          case MP_QSTR_resize: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_resize_obj); destination[1] = self_in; break;
          case MP_QSTR_rotateZ: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotateZ_obj); destination[1] = self_in; break;
          case MP_QSTR_rotateY: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotateY_obj); destination[1] = self_in; break;
          case MP_QSTR_rotateX: destination[0] = MP_OBJ_FROM_PTR(&vector3_class_rotateX_obj); destination[1] = self_in; break;
          default: break;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
          case MP_QSTR_x: self->x = mp_obj_get_float(destination[1]); break;
          case MP_QSTR_y: self->y = mp_obj_get_float(destination[1]); break;
          case MP_QSTR_z: self->z = mp_obj_get_float(destination[1]); break;
          default: return;
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t vector3_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_test), MP_ROM_PTR(&vector3_class_test_obj) },
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
