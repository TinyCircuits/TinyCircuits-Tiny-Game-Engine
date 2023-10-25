#include "rectangle.h"
#include "utility/debug_print.h"

// Class required functions
STATIC void rectangle_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    rectangle_class_obj_t *self = self_in;
    ENGINE_INFO_PRINTF("print(): rectangle [x: %0.3f, y: %0.3f, width: %0.3f, height: %0.3f]", (double)self->x, (double)self->y, (double)self->width, (double)self->height);
}

mp_obj_t rectangle_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New rectangle");
    mp_arg_check_num(n_args, n_kw, 0, 0, true);

    rectangle_class_obj_t *self = m_new_obj(rectangle_class_obj_t);
    self->base.type = &rectangle_class_type;

    self->x = 0.0f;
    self->y = 0.0f;
    self->width = 0.0f;
    self->height = 0.0f;

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t rectangle_class_area(mp_obj_t self){
    ENGINE_INFO_PRINTF("Rectangle area: TODO");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_class_area_obj, rectangle_class_area);


// Function called when accessing like print(my_node.position.x) (load 'x')
// my_node.position.x = 0 (store 'x').
// See https://micropython-usermod.readthedocs.io/en/latest/usermods_09.html#properties
// See https://github.com/micropython/micropython/blob/91a3f183916e1514fbb8dc58ca5b77acc59d4346/extmod/modasyncio.c#L227
STATIC void rectangle_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    rectangle_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        if(attribute == MP_QSTR_x){
            destination[0] = (mp_obj_t*)(&self->x);
        }else if(attribute == MP_QSTR_y){
            destination[0] = (mp_obj_t*)(&self->y);
        }else if(attribute == MP_QSTR_width){
            destination[0] = (mp_obj_t*)(&self->width);
        }else if(attribute == MP_QSTR_height){
            destination[0] = (mp_obj_t*)(&self->height);
        }else if(attribute == MP_QSTR_area){
            destination[0] = MP_OBJ_FROM_PTR(&rectangle_class_area_obj);
            destination[1] = self_in;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        if(attribute == MP_QSTR_x){
            self->x = mp_obj_get_float(destination[1]);
        }else if(attribute == MP_QSTR_y){
            self->y = mp_obj_get_float(destination[1]);
        }else if(attribute == MP_QSTR_width){
            self->width = mp_obj_get_float(destination[1]);
        }else if(attribute == MP_QSTR_height){
            self->height = mp_obj_get_float(destination[1]);
        }else{
            return;
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_area), MP_ROM_PTR(&rectangle_class_area_obj) },
};


// Class init
STATIC MP_DEFINE_CONST_DICT(rectangle_class_locals_dict, rectangle_class_locals_dict_table);

const mp_obj_type_t rectangle_class_type = {
    { &mp_type_type },
    .name = MP_QSTR_Rectangle,
    .print = rectangle_class_print,
    .make_new = rectangle_class_new,
    .call = NULL,
    .unary_op = NULL,
    .binary_op = NULL,
    .attr = rectangle_class_attr,
    .subscr = NULL,
    .getiter = NULL,
    .iternext = NULL,
    .buffer_p = {NULL},
    .locals_dict = (mp_obj_dict_t*)&rectangle_class_locals_dict,
};