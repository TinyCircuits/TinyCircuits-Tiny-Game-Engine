#include "rectangle.h"
#include "debug/debug_print.h"

// Class required functions
STATIC void rectangle_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    (void)kind;
    ENGINE_INFO_PRINTF("print(): Rectangle");
}

mp_obj_t rectangle_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Rectangle");

    rectangle_class_obj_t *self = m_new_obj(rectangle_class_obj_t);
    self->base.type = &rectangle_class_type;

    if(n_args == 0){
        self->x = mp_obj_new_float(0.0f);
        self->y = mp_obj_new_float(0.0f);
        self->width = mp_obj_new_float(0.0f);
        self->height = mp_obj_new_float(0.0f);
    }else if(n_args == 4){
        self->x = args[0];
        self->y = args[1];
        self->width = args[2];
        self->height = args[3];
    }else{
        mp_raise_msg(&mp_type_RuntimeError, "Rectangle: Wrong number of arguments in constructor! Only accepts 0 or 4");
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t rectangle_class_area(mp_obj_t self){
    ENGINE_INFO_PRINTF("Rectangle area: TODO");
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_class_area_obj, rectangle_class_area);


STATIC void rectangle_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    rectangle_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        if(attribute == MP_QSTR_x){
            destination[0] = self->x;
        }else if(attribute == MP_QSTR_y){
            destination[0] = self->y;
        }else if(attribute == MP_QSTR_width){
            destination[0] = self->width;
        }else if(attribute == MP_QSTR_height){
            destination[0] = self->height;
        }else if(attribute == MP_QSTR_area){
            destination[0] = MP_OBJ_FROM_PTR(&rectangle_class_area_obj);
            destination[1] = self_in;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        if(attribute == MP_QSTR_x){
            self->x = destination[1];
        }else if(attribute == MP_QSTR_y){
            self->y = destination[1];
        }else if(attribute == MP_QSTR_width){
            self->width = destination[1];
        }else if(attribute == MP_QSTR_height){
            self->height = destination[1];
        }else{
            return;
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_class_locals_dict_table[] = {

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