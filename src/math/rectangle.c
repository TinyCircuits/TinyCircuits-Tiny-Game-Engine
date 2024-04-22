#include "rectangle.h"
#include "debug/debug_print.h"


mp_obj_t rectangle_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Rectangle");

    rectangle_class_obj_t *self = m_new_obj(rectangle_class_obj_t);
    self->base.type = &rectangle_class_type;

    if(n_args == 0){
        self->x = 0.0f;
        self->y = 0.0f;
        self->width = 0.0f;
        self->height = 0.0f;
    }else if(n_args == 4){
        self->x = mp_obj_get_float(args[0]);
        self->y = mp_obj_get_float(args[1]);
        self->width = mp_obj_get_float(args[2]);
        self->height = mp_obj_get_float(args[3]);
    }else{
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Rectangle: Wrong number of arguments in constructor! Only accepts 0 or 4"));
    }

    return MP_OBJ_FROM_PTR(self);
}


// Class methods
STATIC mp_obj_t rectangle_class_area(mp_obj_t self_in){
    rectangle_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    ENGINE_INFO_PRINTF("Rectangle getting area...");
    return mp_obj_new_float(self->width * self->height);
}
MP_DEFINE_CONST_FUN_OBJ_1(rectangle_class_area_obj, rectangle_class_area);


STATIC mp_obj_t rectangle_class_overlapping(mp_obj_t self_in, mp_obj_t b_in){
    rectangle_class_obj_t* self = MP_OBJ_TO_PTR(self_in);
    rectangle_class_obj_t* b = MP_OBJ_TO_PTR(b_in);
    if(b->x + b->width < self->x) return mp_const_false;
    else if(self->x + self->width < b->x) return mp_const_false;
    else if(b->y + b->height < self->y) return mp_const_false;
    else if(self->y + self->height < b->y) return mp_const_false;
    else return mp_const_true;
}
MP_DEFINE_CONST_FUN_OBJ_2(rectangle_class_overlapping_obj, rectangle_class_overlapping);


/*  --- doc ---
    NAME: Rectangle
    DESC: Holds an X and Y plus width and height
    PARAM:  [type=float]  [name=x]      [value=any]
    PARAM:  [type=float]  [name=y]      [value=any]
    PARAM:  [type=float]  [name=width]  [value=any]
    PARAM:  [type=float]  [name=height] [value=any]                                                                                   
    ATTR:   [type=float]  [name=x]      [value=any]                                
    ATTR:   [type=float]  [name=y]      [value=any]
    ATTR:   [type=float]  [name=width]  [value=any]
    ATTR:   [type=float]  [name=height] [value=any]                                                             
*/ 
STATIC void rectangle_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    rectangle_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_x:
                destination[0] = mp_obj_new_float(self->x);
            break;
            case MP_QSTR_y:
                destination[0] = mp_obj_new_float(self->y);
            break;
            case MP_QSTR_width:
                destination[0] = mp_obj_new_float(self->width);
            break;
            case MP_QSTR_height:
                destination[0] = mp_obj_new_float(self->height);
            break;
            case MP_QSTR_area:
                destination[0] = MP_OBJ_FROM_PTR(&rectangle_class_area_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_overlapping:
                destination[0] = MP_OBJ_FROM_PTR(&rectangle_class_overlapping_obj);
                destination[1] = self_in;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute){
            case MP_QSTR_x:
                self->x = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_y:
                self->y = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_width:
                self->width = mp_obj_get_float(destination[1]);
            break;
            case MP_QSTR_height:
                self->height = mp_obj_get_float(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t rectangle_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(rectangle_class_locals_dict, rectangle_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    rectangle_class_type,
    MP_QSTR_Rectangle,
    MP_TYPE_FLAG_NONE,

    make_new, rectangle_class_new,
    attr, rectangle_class_attr,
    locals_dict, &rectangle_class_locals_dict
);