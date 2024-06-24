#include "engine_color.h"
#include "debug/debug_print.h"
#include "utility/engine_defines.h"
#include "math/engine_math.h"

const uint16_t bitmask_5_bit = 0b0000000000011111;
const uint16_t bitmask_6_bit = 0b0000000000111111;

static inline float clamp_0_to_1(float value) {
    return engine_math_clamp(value, 0.0f, 1.0f);
}

static inline uint16_t round_float(float value){
    return (uint16_t)(value + 0.5f);
}

static inline void engine_color_split_u16(uint16_t color, uint16_t *r, uint16_t *g, uint16_t *b){
    *r = (color >> 11) & bitmask_5_bit;
    *g = (color >>  5) & bitmask_6_bit;
    *b = (color >>  0) & bitmask_5_bit;
}

// https://stackoverflow.com/a/29321264
uint16_t ENGINE_FAST_FUNCTION(engine_color_blend)(uint16_t from, uint16_t to, float amount){
    uint16_t from_r, from_g, from_b;
    engine_color_split_u16(from, &from_r, &from_g, &from_b);
    uint16_t to_r, to_g, to_b;
    engine_color_split_u16(to, &to_r, &to_g, &to_b);

    const uint16_t out_r = round_float(clamp_0_to_1(sqrtf((1.0f - amount) * (from_r*from_r) + amount * (to_r*to_r))));
    const uint16_t out_g = round_float(clamp_0_to_1(sqrtf((1.0f - amount) * (from_g*from_g) + amount * (to_g*to_g))));
    const uint16_t out_b = round_float(clamp_0_to_1(sqrtf((1.0f - amount) * (from_b*from_b) + amount * (to_b*to_b))));

    return (out_r << 11) | (out_g << 5) | (out_b << 0);
}


// https://stackoverflow.com/a/19060243
uint16_t ENGINE_FAST_FUNCTION(engine_color_alpha_blend)(uint16_t background, uint16_t foreground, float alpha){
    uint16_t bg_r, bg_g, bg_b;
    engine_color_split_u16(background, &bg_r, &bg_g, &bg_b);
    uint16_t fg_r, fg_g, fg_b;
    engine_color_split_u16(foreground, &fg_r, &fg_g, &fg_b);

    const uint16_t out_r = round_float((fg_r * alpha + bg_r * (1.0f-alpha)));
    const uint16_t out_g = round_float((fg_g * alpha + bg_g * (1.0f-alpha)));
    const uint16_t out_b = round_float((fg_b * alpha + bg_b * (1.0f-alpha)));

    return (out_r << 11) | (out_g << 5) | (out_b << 0);
}


static void color_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    color_class_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint16_t r, g, b;
    engine_color_split_u16(self->value, &r, &g, &b);
    ENGINE_PRINTF("(%.04f %.04f %.04f)", r / (double)bitmask_5_bit, g / (double)bitmask_6_bit, b / (double)bitmask_5_bit);
}


uint16_t engine_color_class_color_value(mp_obj_t color) {
    if (MP_OBJ_IS_TYPE(color, &color_class_type) || MP_OBJ_IS_TYPE(color, &const_color_class_type)) {
        return ((color_class_obj_t*)MP_OBJ_TO_PTR(color))->value;
    } else if(MP_OBJ_IS_INT(color)) {
        return mp_obj_get_int(color);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Color: Expected Color or int (RGB565)"));
    }
}


mp_obj_t engine_color_wrap(mp_obj_t color) {
    if (MP_OBJ_IS_TYPE(color, &color_class_type) || MP_OBJ_IS_TYPE(color, &const_color_class_type)) {
        return color;
    } else if(MP_OBJ_IS_INT(color)) {
        color_class_obj_t *color_obj = mp_obj_malloc(color_class_obj_t, &color_class_type);
        color_obj->value = mp_obj_get_int(color);
        return MP_OBJ_FROM_PTR(color_obj);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Color: Expected Color or int (RGB565)"));
    }
}


mp_obj_t engine_color_wrap_opt(mp_obj_t color_or_none) {
    if (color_or_none == mp_const_none) {
        return mp_const_none;
    } else {
        return engine_color_wrap(color_or_none);
    }
}


/*  --- doc ---
    NAME: Color
    ID: Color
    DESC: Simple object that holds an RGB color. Normal of constructing this is by passing 3 channels, but you can also pass it nothing to default to black or a single int in RGB565, or Color.
    PARAM:  [type=float]    [name=r]       [value=0.0 ~ 1.0]
    PARAM:  [type=float]    [name=g]       [value=0.0 ~ 1.0]
    PARAM:  [type=float]    [name=b]       [value=0.0 ~ 1.0]
    ATTR:   [type=float]    [name=r]       [value=0.0 ~ 1.0]
    ATTR:   [type=float]    [name=g]       [value=0.0 ~ 1.0]
    ATTR:   [type=float]    [name=b]       [value=0.0 ~ 1.0]
    ATTR:   [type=int]      [name=value]   [value=int RGB565]
*/
mp_obj_t color_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New Color");
    color_class_obj_t *self = mp_obj_malloc(color_class_obj_t, &color_class_type);
    if (n_args == 0) {
        self->value = 0;
    } else if (n_args == 1) {
        self->value = engine_color_class_color_value(args[0]);
    } else if (n_args == 3) {
        self->value = engine_color_from_rgb_float(mp_obj_get_float(args[0]), mp_obj_get_float(args[1]), mp_obj_get_float(args[2]));
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("Color: ERROR: expected 0, 1 or 3 arguments"));
    }
    return MP_OBJ_FROM_PTR(self);
}

inline float engine_color_get_r_float(uint16_t color){
    return ((color >> 11) & bitmask_5_bit) / (float)bitmask_5_bit;
}
inline float engine_color_get_g_float(uint16_t color){
    return ((color >> 5) & bitmask_6_bit) / (float)bitmask_6_bit;
}
inline float engine_color_get_b_float(uint16_t color){
    return ((color >> 0) & bitmask_5_bit) / (float)bitmask_5_bit;
}

uint16_t engine_color_set_r_float(uint16_t color, float r){
    return (color & ~(bitmask_5_bit << 11)) | (round_float(clamp_0_to_1(r) * bitmask_5_bit) << 11);
}
uint16_t engine_color_set_g_float(uint16_t color, float g){
    return (color & ~(bitmask_6_bit << 5)) | (round_float(clamp_0_to_1(g) * bitmask_6_bit) << 5);
}
uint16_t engine_color_set_b_float(uint16_t color, float b){
    return (color & ~(bitmask_5_bit << 0)) | (round_float(clamp_0_to_1(b) * bitmask_5_bit) << 0);
}

uint16_t engine_color_from_rgb_float(float r, float g, float b){
    return
        (round_float(clamp_0_to_1(r) * bitmask_5_bit) << 11) |
        (round_float(clamp_0_to_1(g) * bitmask_6_bit) << 5) |
        (round_float(clamp_0_to_1(b) * bitmask_5_bit) << 0);
}


static mp_obj_t engine_color_set(size_t n_args, const mp_obj_t *args) {
    color_class_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    self->value = engine_color_from_rgb_float(mp_obj_get_float(args[1]), mp_obj_get_float(args[2]), mp_obj_get_float(args[3]));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_color_set_obj, 4, 4, engine_color_set);


static void color_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing Color attr");

    color_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_r:
                destination[0] = mp_obj_new_float(engine_color_get_r_float(self->value));
            break;
            case MP_QSTR_g:
                destination[0] = mp_obj_new_float(engine_color_get_g_float(self->value));
            break;
            case MP_QSTR_b:
                destination[0] = mp_obj_new_float(engine_color_get_b_float(self->value));
            break;
            case MP_QSTR_value:
                destination[0] = MP_OBJ_NEW_SMALL_INT(self->value);
            break;
            default:
                // Continue in locals_dict.
                destination[1] = MP_OBJ_SENTINEL;
                return;
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        if(self->base.type == &const_color_class_type){
            return; // Fail
        }
        switch(attribute) {
            case MP_QSTR_r:
                self->value = engine_color_set_r_float(self->value, mp_obj_get_float(destination[1]));
            break;
            case MP_QSTR_g:
                self->value = engine_color_set_g_float(self->value, mp_obj_get_float(destination[1]));
            break;
            case MP_QSTR_b:
                self->value = engine_color_set_b_float(self->value, mp_obj_get_float(destination[1]));
            break;
            case MP_QSTR_value:
                self->value = mp_obj_get_int(destination[1]);
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


const mp_rom_map_elem_t color_class_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_set), MP_ROM_PTR(&engine_color_set_obj) },
};
MP_DEFINE_CONST_DICT(color_class_locals_dict, color_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    color_class_type,
    MP_QSTR_Color,
    MP_TYPE_FLAG_NONE,

    make_new, color_class_new,
    attr, color_class_attr,
    locals_dict, (mp_obj_dict_t*)&color_class_locals_dict,
    print, color_class_print
);

MP_DEFINE_CONST_OBJ_TYPE(
    const_color_class_type,
    MP_QSTR_ConstColor,
    MP_TYPE_FLAG_NONE,

    attr, color_class_attr,
    print, color_class_print
);
