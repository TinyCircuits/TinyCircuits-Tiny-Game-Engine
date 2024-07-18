#ifndef ENGINE_FONT_RESOURCE_H
#define ENGINE_FONT_RESOURCE_H

#include "py/obj.h"
#include "resources/engine_texture_resource.h"

// Default mapping is: ` !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~`
// where glyph_widths[0] refers to width of the space character, for example
#define ENGINE_FONT_MAX_CHAR_COUNT 126 - 32

typedef struct{
    mp_obj_base_t base;
    texture_resource_class_obj_t *texture_resource;

    uint8_t glyph_widths[ENGINE_FONT_MAX_CHAR_COUNT];
    uint16_t glyph_x_offsets[ENGINE_FONT_MAX_CHAR_COUNT];

    mp_obj_t glyph_widths_bytearray_ref;
    mp_obj_t glyph_offsets_bytearray_ref;
    uint8_t glyph_height;
}font_resource_class_obj_t;

extern font_resource_class_obj_t font;

extern const mp_obj_type_t font_resource_class_type;

mp_obj_t font_resource_class_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args);
uint8_t font_resource_get_glyph_width(font_resource_class_obj_t *font, char codepoint);
uint16_t font_resource_get_glyph_x_offset(font_resource_class_obj_t *font, char codepoint);
void font_resource_get_box_dimensions(font_resource_class_obj_t *font, mp_obj_t text, float *text_box_width, float *text_box_height, float letter_spacing, float line_spacing);

#endif  // ENGINE_FONT_RESOURCE_H