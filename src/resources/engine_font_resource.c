#include "engine_font_resource.h"
#include "debug/debug_print.h"
#include "math/engine_math.h"
#include "py/objtype.h"
#include "py/objstr.h"


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
    uint32_t bitmap_height = self->texture_resource->height;

    // The bottom row of pixels is used for defining the width
    // of each character using alternating colors
    self->glyph_height = bitmap_height-1;

    // Used to traverse pixels in bottom row to determine character widths
    uint16_t alternating_pixel_x = 0;
    uint16_t alternating_pixel_y = self->glyph_height;

    // Start tracking the initial width color, what 
    // character we're on, and set the first offset to 0
    uint16_t last_width_signifier_color = texture_resource_get_pixel(self->texture_resource, engine_math_2d_to_1d_index(alternating_pixel_x, alternating_pixel_y, bitmap_width));
    uint16_t current_glyph_index = 0;

    // Loop until end of bitmap width is reached
    while(true){
        // Increase to get the next pixel
        alternating_pixel_x++;

        // If at the end of the bitmap, error if did not collect
        // enough chars, otherwise, end loop
        if(alternating_pixel_x >= bitmap_width){
            if(current_glyph_index != ENGINE_FONT_MAX_CHAR_COUNT){
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineFont: Did not collect enough character entries in font BMP!"));
            }else{
                break;
            }
        }

        // Get the next pixel, and since we got another pixel,
        // increase the width of the current character
        uint16_t next_width_signifier_color = texture_resource_get_pixel(self->texture_resource, engine_math_2d_to_1d_index(alternating_pixel_x, self->glyph_height, bitmap_width));
        self->glyph_widths[current_glyph_index]++;

        // If the pixel to the left of this one is not the same, 
        // we're moving on to a new character
        if(last_width_signifier_color != next_width_signifier_color){
            current_glyph_index++;
            last_width_signifier_color = next_width_signifier_color;
            self->glyph_x_offsets[current_glyph_index] = alternating_pixel_x;
        }
    }

    // First offset always at 0
    self->glyph_x_offsets[0] = 0;

    self->glyph_widths_bytearray_ref = mp_obj_new_bytearray_by_ref(ENGINE_FONT_MAX_CHAR_COUNT, self->glyph_widths);

    return MP_OBJ_FROM_PTR(self);
}


uint8_t font_resource_get_glyph_width(font_resource_class_obj_t *font, char codepoint){
    // ASCII space is 32 but mapped to index 0 in the array
    return font->glyph_widths[codepoint - 32];
}


uint16_t font_resource_get_glyph_x_offset(font_resource_class_obj_t *font, char codepoint){
    // ASCII space is 32 but mapped to index 0 in the array
    return font->glyph_x_offsets[codepoint - 32];
}


void font_resource_get_box_dimensions(font_resource_class_obj_t *font, mp_obj_t text, float *text_box_width, float *text_box_height){
    // Get length of string: https://github.com/v923z/micropython-usermod/blob/master/snippets/stringarg/stringarg.c
    GET_STR_DATA_LEN(text, str, str_len);

    // Figure out the size of the text box, considering newlines
    *text_box_width = 0.0f;
    *text_box_height = font->glyph_height;
    float temp_text_box_width = 0.0f;
    for(uint16_t icx=0; icx<str_len; icx++){
        char current_char = ((char *)str)[icx];

        // Check if newline, otherwise any other character contributes to text box width
        if(current_char == 10){
            *text_box_height += font->glyph_height;
            temp_text_box_width = 0.0f;
        }else{
            temp_text_box_width += font_resource_get_glyph_width(font, current_char);
        }

        // Trying to find row with the most width
        // which will define total text box size
        if(temp_text_box_width > *text_box_width){
            *text_box_width = temp_text_box_width;
        }
    }
}


// Class methods
STATIC mp_obj_t font_resource_class_del(mp_obj_t self_in){
    ENGINE_INFO_PRINTF("FontResource: Deleted");

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(font_resource_class_del_obj, font_resource_class_del);


/*  --- doc ---
    NAME: FontResource
    DESC: Object that holds information about a font that can be used in {ref_link:Text2DNode} to display text. The file needs to be a 16-bit RGB565 .bmp file consisting of characters all of the same height. Widths of the characters are marked by any alternating colors in the bottom row of pixels of the bitmap. Characters should be in one large row.
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