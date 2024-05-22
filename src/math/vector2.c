#include "vector2.h"
#include "debug/debug_print.h"
#include "math/engine_math.h"
#include "utility/engine_mp.h"


STATIC void vector2_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_PRINTF("%.15f %.15f", ((mp_obj_float_t)((vector2_class_obj_t*)self_in)->x).value, ((mp_obj_float_t)((vector2_class_obj_t*)self_in)->y).value);
}


mp_obj_t vector2_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Vector2");
    vector2_class_obj_t *self = m_new_obj(vector2_class_obj_t);

    self->base.type = &vector2_class_type;
    self->on_change_user_ptr = NULL;
    self->on_changing = NULL;
    self->on_changed = NULL;

    self->x.base.type = &mp_type_float;
    self->y.base.type = &mp_type_float;

    if(n_args == 0){
        self->x.value = 0.0f;
        self->y.value = 0.0f;
    }else if(n_args == 2){
        self->x.value = mp_obj_get_float(args[0]);
        self->y.value = mp_obj_get_float(args[1]);
    }else{
        mp_raise_TypeError(MP_ERROR_TEXT("function takes 0 or 2 arguments"));
    }

    return MP_OBJ_FROM_PTR(self);
}


/*  --- doc ---
    NAME: length
    ID: vector2_length
    DESC: Calculates and returns length of the {ref_link:Vector2}                                                                                                        
    RETURN: float
*/ 
mp_obj_t vector2_class_length(mp_obj_t self_in){
    vector2_class_obj_t *self = self_in;

    return mp_obj_new_float(engine_math_vector_length(self->x.value, self->y.value));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(vector2_class_length_obj, vector2_class_length);


/*  --- doc ---
    NAME: normalized
    ID: vector2_normalized
    DESC: Calculates and returns normalized verion of the {ref_link:Vector2}                                                                                                        
    RETURN: {ref_link:Vector2}
*/ 
mp_obj_t vector2_class_normalized(mp_obj_t self_in){
    vector2_class_obj_t *self = self_in;

    float normalized_x = self->x.value;
    float normalized_y = self->y.value;

    engine_math_normalize(&normalized_x, &normalized_y);

    return vector2_class_new(&vector2_class_type, 2, 0, (mp_obj_t[]){mp_obj_new_float(normalized_x), mp_obj_new_float(normalized_y)}); 
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(vector2_class_normalized_obj, vector2_class_normalized);


/*  --- doc ---
    NAME: Vector2
    ID: Vector2
    DESC: Holds an X and Y value. Typically used for position
    PARAM:  [type=float]        [name=x]                                [value=any]
    PARAM:  [type=float]        [name=y]                                [value=any]                                                                                         
    ATTR:   [type=float]        [name=x]                                [value=any]                                
    ATTR:   [type=float]        [name=y]                                [value=any]
    ATTR:   [type=function]     [name={ref_link:vector2_length}]        [value=function]
    ATTR:   [type=function]     [name={ref_link:vector2_normalized}]    [value=function]                                                                     
*/ 
STATIC void vector2_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Vector2 attr");

    vector2_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute){
            case MP_QSTR_length:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_length_obj);
                destination[1] = self;
            break;
            case MP_QSTR_normalized:
                destination[0] = MP_OBJ_FROM_PTR(&vector2_class_normalized_obj);
                destination[1] = self;
            break;
            case MP_QSTR_x:
                destination[0] = mp_obj_new_float(self->x.value);
            break;
            case MP_QSTR_y:
                destination[0] = mp_obj_new_float(self->y.value);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            // When `x` or `y` is changed call the `on_change` function in case something is watching for changes
            case MP_QSTR_x:
            {
                float new_x = mp_obj_get_float(destination[1]);
                if(self->on_changing != NULL) self->on_changing(self->on_change_user_ptr, new_x, self->y.value);
                self->x.value = new_x;
                if(self->on_changed != NULL) self->on_changed(self->on_change_user_ptr);
            }
            break;
            case MP_QSTR_y:
            {
                float new_y = mp_obj_get_float(destination[1]);
                if(self->on_changing != NULL) self->on_changing(self->on_change_user_ptr, self->x.value, new_y);
                self->y.value = mp_obj_get_float(destination[1]);
                if(self->on_changed != NULL)self->on_changed(self->on_change_user_ptr);
            }
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
STATIC MP_DEFINE_CONST_DICT(vector2_class_locals_dict, vector2_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    vector2_class_type,
    MP_QSTR_Vector2,
    MP_TYPE_FLAG_NONE,

    make_new, vector2_class_new,
    attr, vector2_class_attr,
    print, vector2_class_print,
    locals_dict, &vector2_class_locals_dict
);