#include "engine_color.h"
#include "debug/debug_print.h"
#include "utility/engine_defines.h"
#include "math/engine_math.h"

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
    const uint16_t r = (uint16_t)(color->r.value * (float)bitmask_5_bit);
    const uint16_t g = (uint16_t)(color->g.value * (float)bitmask_6_bit);
    const uint16_t b = (uint16_t)(color->b.value * (float)bitmask_5_bit);

    color->value.val = 0;
    color->value.val |= (r << 11);
    color->value.val |= (g << 5);
    color->value.val |= (b << 0);
}


// https://stackoverflow.com/a/29321264
uint16_t ENGINE_FAST_FUNCTION(engine_color_blend)(uint16_t from, uint16_t to, float amount){
    const uint16_t from_r = (from >> 11) & bitmask_5_bit;
    const uint16_t from_g = (from >> 5)  & bitmask_6_bit;
    const uint16_t from_b = (from >> 0)  & bitmask_5_bit;

    const uint16_t to_r = (to >> 11) & bitmask_5_bit;
    const uint16_t to_g = (to >> 5)  & bitmask_6_bit;
    const uint16_t to_b = (to >> 0)  & bitmask_5_bit;

    const uint16_t out_r = (uint16_t)sqrtf((1.0f - amount) * (from_r*from_r) + amount * (to_r*to_r));
    const uint16_t out_g = (uint16_t)sqrtf((1.0f - amount) * (from_g*from_g) + amount * (to_g*to_g));
    const uint16_t out_b = (uint16_t)sqrtf((1.0f - amount) * (from_b*from_b) + amount * (to_b*to_b));

    uint16_t result = 0;
    result |= (out_r << 11);
    result |= (out_g << 5);
    result |= (out_b << 0);

    return result;
}


// https://stackoverflow.com/a/19060243
uint16_t ENGINE_FAST_FUNCTION(engine_color_alpha_blend)(uint16_t background, uint16_t foreground, float alpha){
    const uint16_t bg_r = (background >> 11) & bitmask_5_bit;
    const uint16_t bg_g = (background >> 5)  & bitmask_6_bit;
    const uint16_t bg_b = (background >> 0)  & bitmask_5_bit;

    const uint16_t fg_r = (foreground >> 11) & bitmask_5_bit;
    const uint16_t fg_g = (foreground >> 5)  & bitmask_6_bit;
    const uint16_t fg_b = (foreground >> 0)  & bitmask_5_bit;

    const uint16_t out_r = (uint16_t)(fg_r * alpha + bg_r * (1.0f-alpha));
    const uint16_t out_g = (uint16_t)(fg_g * alpha + bg_g * (1.0f-alpha));
    const uint16_t out_b = (uint16_t)(fg_b * alpha + bg_b * (1.0f-alpha));

    uint16_t result = 0;

    // Clamp color values to maximum
    result |= (out_r & bitmask_5_bit) << 11;
    result |= (out_g & bitmask_6_bit) << 5;
    result |= (out_b & bitmask_5_bit) << 0;

    return result;
}


STATIC void color_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    double r = (double)(((mp_obj_float_t)((color_class_obj_t*)self_in)->r).value);
    double g = (double)(((mp_obj_float_t)((color_class_obj_t*)self_in)->g).value);
    double b = (double)(((mp_obj_float_t)((color_class_obj_t*)self_in)->b).value);
    ENGINE_PRINTF("%.09f %.09f %.09f", r, g, b);
}


/*  --- doc ---
    NAME: Color
    ID: Color
    DESC: Simple object that holds an RGB color. Normal of constructing this is by passing 3 channels, but you can also pass it nothing to default to black or a single int in RGB565
    PARAM:  [type=float]    [name=r]   [value=0.0 ~ 1.0]
    PARAM:  [type=float]    [name=g]   [value=0.0 ~ 1.0]
    PARAM:  [type=float]    [name=b]   [value=0.0 ~ 1.0]
    ATTR:   [type=float]    [name=r]   [value=0.0 ~ 1.0]
    ATTR:   [type=float]    [name=g]   [value=0.0 ~ 1.0]
    ATTR:   [type=float]    [name=b]   [value=0.0 ~ 1.0]
*/
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
        engine_color_sync_u16_to_rgb(self);
    }else if(n_args == 1){
        self->value.val = mp_obj_get_int(args[0]);
        engine_color_sync_u16_to_rgb(self);
    }else if(n_args == 3){
        self->r.value = mp_obj_get_float(args[0]);
        self->g.value = mp_obj_get_float(args[1]);
        self->b.value = mp_obj_get_float(args[2]);
        engine_color_sync_rgb_to_u16(self);
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
                destination[0] = mp_obj_new_float(self->r.value);
            break;
            case MP_QSTR_g:
                destination[0] = mp_obj_new_float(self->g.value);
            break;
            case MP_QSTR_b:
                destination[0] = mp_obj_new_float(self->b.value);
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
    attr, color_class_attr,
    print, color_class_print,
    locals_dict, &color_class_locals_dict
);