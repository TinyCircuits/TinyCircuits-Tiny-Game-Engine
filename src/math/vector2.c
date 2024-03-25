#include "vector2.h"
#include "debug/debug_print.h"

// Class required functions
STATIC void vector2_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    vector2_class_obj_t *self = self_in;
    ENGINE_FORCE_PRINTF("print(): Vector2 [%0.3f, %0.3f]", (double)self->x.value, (double)self->y.value);
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
        self->base.type = &vector2_class_type;
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
    NAME: Vector2
    DESC: Holds an X and Y value. Typically used for position
    PARAM: [type=float]  [name=x]    [value=any]
    PARAM: [type=float]  [name=y]    [value=any]                                                                                         
    ATTR: [type=float]  [name=x]    [value=any]                                
    ATTR: [type=float]  [name=y]    [value=any]                                                                             
*/ 
STATIC void vector2_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Vector2 attr");

    vector2_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_x:
                destination[0] = &self->x;
            break;
            case MP_QSTR_y:
                destination[0] = &self->y;
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
    print, vector2_class_print,
    attr, vector2_class_attr,
    locals_dict, &vector2_class_locals_dict
);