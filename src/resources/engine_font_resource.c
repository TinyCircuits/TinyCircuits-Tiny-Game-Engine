#include "engine_font_resource.h"
#include "debug/debug_print.h"
#include "math/engine_math.h"


// Class required functions
STATIC void font_resource_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_INFO_PRINTF("print(): FontResource");
}


mp_obj_t font_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args){
    ENGINE_INFO_PRINTF("New FontResource");

    // Need at least the path to the font BMP,
    // but also accepts flag indicating if in ram
    // TODO: maybe in the case that no file path
    // is passed, use a default compiled in font?
    mp_arg_check_num(n_args, n_kw, 1, 2, false);

    font_resource_class_obj_t *self = m_new_obj_with_finaliser(font_resource_class_obj_t);
    self->base.type = &font_resource_class_type;
    self->texture_resource = texture_resource_class_new(&texture_resource_class_type, n_args, 0, args);

    uint32_t bitmap_width = self->texture_resource->width;

    // The bottom row of pixels is used for defining the width
    // of each character using alternating colors
    self->glyph_height = self->texture_resource->height-1;

    uint16_t current_pixel_index = 0;
    uint16_t last_width_signifier_color = texture_resource_get_pixel(self->texture_resource, engine_math_2d_to_1d_index(current_pixel_index, self->glyph_height, bitmap_width));
    uint16_t glyph_index = 0;
    self->glyph_x_offsets[0] = 0;

    while(glyph_index < ENGINE_FONT_MAX_CHAR_COUNT){
        current_pixel_index++;

        uint16_t next_width_signifier_color = texture_resource_get_pixel(self->texture_resource, engine_math_2d_to_1d_index(current_pixel_index, self->glyph_height, bitmap_width));

        self->glyph_widths[glyph_index]++;

        if(last_width_signifier_color != next_width_signifier_color){
            glyph_index++;
            last_width_signifier_color = next_width_signifier_color;
            self->glyph_x_offsets[glyph_index] = current_pixel_index+1;
        }
        
        // When we get to the end of the font bitmap,
        // make sure we collected enough entries for
        // all characters, otherwise error out
        if(current_pixel_index >= self->texture_resource->width){
            if(glyph_index != ENGINE_FONT_MAX_CHAR_COUNT){
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineFont: Did not collect enough character entries in font BMP!"));
            }
        }
    }

    self->glyph_widths_bytearray_ref = mp_obj_new_bytearray_by_ref(ENGINE_FONT_MAX_CHAR_COUNT, self->glyph_widths);

    return MP_OBJ_FROM_PTR(self);
}


uint8_t font_resource_get_glyph_width(font_resource_class_obj_t *font, char codepoint){
    // ASCII space is 32 but mapped to index 0 in the array
    return font->glyph_widths[codepoint - 33];
}


uint8_t font_resource_get_glyph_x_offset(font_resource_class_obj_t *font, char codepoint){
    // ASCII space is 32 but mapped to index 0 in the array
    return font->glyph_x_offsets[codepoint - 33];
}


// Class methods
STATIC mp_obj_t font_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("FontResource: Deleted");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(font_resource_class_del_obj, font_resource_class_del);


/*  --- doc ---
    NAME: FontResource
    DESC: Object that holds information about a font
    PARAM:  [type=string]                       [name=filepath] [value=string]
    PARAM:  [type=boolean]                      [name=in_ram]   [value=True of False (False by default)]
    ATTR:   [type={ref_link:TextureResource}]   [name=texture]  [value={ref_link:TextureResource}]
    ATTR:   [type=bytearray]                    [name=widths]   [value=bytearray (read-only)]
    ATTR:   [type=int]                          [name=height]   [value=any (read-only)]
*/ 
STATIC void font_resource_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    ENGINE_INFO_PRINTF("Accessing FontResource attr");

    font_resource_class_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR___del__:
                destination[0] = MP_OBJ_FROM_PTR(&font_resource_class_del_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_texture:
                destination[0] = self->texture_resource;
            break;
            case MP_QSTR_height:
                destination[0] = mp_obj_new_int(self->glyph_height);
            break;
            case MP_QSTR_widths:
                destination[0] = self->glyph_widths_bytearray_ref;
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_texture:
                self->texture_resource = destination[1];
            break;
            default:
                return; // Fail
        }

        // Success
        destination[0] = MP_OBJ_NULL;
    }
}


// Class attributes
STATIC const mp_rom_map_elem_t font_resource_class_locals_dict_table[] = {

};
STATIC MP_DEFINE_CONST_DICT(font_resource_class_locals_dict, font_resource_class_locals_dict_table);


MP_DEFINE_CONST_OBJ_TYPE(
    font_resource_class_type,
    MP_QSTR_FontResource,
    MP_TYPE_FLAG_NONE,

    make_new, font_resource_class_new,
    print, font_resource_class_print,
    attr, font_resource_class_attr,
    locals_dict, &font_resource_class_locals_dict
);