#include "engine_color.h"
#include "debug/debug_print.h"

const uint16_t bitmask_5_bit = 0b0000000000011111;
const uint16_t bitmask_6_bit = 0b0000000000111111;

void engine_color_sync_u16_to_rgb(color_class_obj_t *color){
    // RGB565
    const uint16_t r = (color->value.val >> 11) & bitmask_5_bit;
    const uint16_t g = (color->value.val >> 5)  & bitmask_6_bit;
    const uint16_t b = (color->value.val >> 0)  & bitmask_5_bit;

    // RGB1.0,1.0,1.0
    color->r.value = (float)r / (float)bitmask_5_bit;
    color->g.value = (float)g / (float)bitmask_6_bit;
    color->b.value = (float)b / (float)bitmask_5_bit;
}


void engine_color_sync_rgb_to_u16(color_class_obj_t *color){
    const uint16_t r = color->r.value * (float)bitmask_5_bit;
    const uint16_t g = color->g.value * (float)bitmask_6_bit;
    const uint16_t b = color->b.value * (float)bitmask_5_bit;

    color->value.val = 0;
    color->value.val |= (r << 11);
    color->value.val |= (g << 5);
    color->value.val |= (b << 0);
}



// Class required functions
STATIC void color_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    color_class_obj_t *self = self_in;
    ENGINE_PRINTF("print(): Color [%0.3f, %0.3f, %0.3f]", self->r.value, self->g.value, self->b.value);
}


mp_obj_t color_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Color");
    color_class_obj_t *self = m_new_obj(color_class_obj_t);
    self->base.type = &color_class_type;

    self->r.base.type = &mp_type_float;
    self->g.base.type = &mp_type_float;
    self->b.base.type = &mp_type_float;
    self->value.base.type = &mp_type_int;

    if(n_args == 0) {
        self->r.value = 0.0f;
        self->g.value = 0.0f;
        self->b.value = 0.0f;
    }else if(n_args == 1){
        self->value.val = mp_obj_get_int(args[0]);
        engine_color_sync_u16_to_rgb(self);
    }else if(n_args == 3){
        self->r.value = mp_obj_get_float(args[0]);
        self->g.value = mp_obj_get_float(args[1]);
        self->b.value = mp_obj_get_float(args[2]);
    }else{
        mp_raise_TypeError(MP_ERROR_TEXT("Color: ERROR: expected 0 or 3 arguments"));
    }
    
    return MP_OBJ_FROM_PTR(self);
}


STATIC void color_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Color attr");

    color_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_r:
                destination[0] = &self->r;
            break;
            case MP_QSTR_g:
                destination[0] = &self->g;
            break;
            case MP_QSTR_b:
                destination[0] = &self->b;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_r:
                self->r.value = mp_obj_get_float(destination[1]);
                engine_color_sync_rgb_to_u16(self);
            break;
            case MP_QSTR_g:
                self->g.value = mp_obj_get_float(destination[1]);
                engine_color_sync_rgb_to_u16(self);
            break;
            case MP_QSTR_b:
                self->b.value = mp_obj_get_float(destination[1]);
                engine_color_sync_rgb_to_u16(self);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t color_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(color_class_locals_dict, color_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    color_class_type,
    MP_QSTR_Color,
    MP_TYPE_FLAG_NONE,

    make_new, color_class_new,
    print, color_class_print,
    attr, color_class_attr,
    locals_dict, &color_class_locals_dict
);