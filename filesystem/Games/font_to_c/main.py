import engine_main

from engine_resources import FontResource

font = FontResource("font5x7.bmp")

print("")

# Print bytearray object
data = ""
for i in range(0, len(font.texture.data), 1):
    if i < len(font.texture.data)-1:
        data += str(font.texture.data[i]) + ","
    else:
        data += str(font.texture.data[i])

texture_data_str = ""
texture_data_str += "mp_obj_array_t font_texture_data = {\n"
texture_data_str += "  .base.type = &mp_type_bytearray,\n"
texture_data_str += "  .typecode = BYTEARRAY_TYPECODE,\n"
texture_data_str += "  .free = 0,\n"
texture_data_str += "  .len = " + str(font.texture.width*font.texture.height*2) + ",\n"
texture_data_str += "  .items = (uint8_t[]){ " + data + " }\n"
texture_data_str += "};\n"
print(texture_data_str)

print("")

# Print texture resource object
texture_str = ""
texture_str += "texture_resource_class_obj_t font_texture = {\n"
texture_str += "  .base.type = &texture_resource_class_type,\n"
texture_str += "  .in_ram = false,\n"
texture_str += "  .width = " + str(font.texture.width) + ",\n"
texture_str += "  .height = " + str(font.texture.height) + ",\n"
texture_str += "  .data = &font_texture_data\n"
texture_str += "};\n"
print(texture_str)

print("")

widths = ""
for i in range(0, len(font.widths), 1):
    if i < len(font.widths)-1:
        widths += str(font.widths[i]) + ","
    else:
        widths += str(font.widths[i])

offsets = ""
for i in range(0, len(font.offsets), 2):
    value = 0
    value = value | font.offsets[i+1] << 8
    value = value | font.offsets[i] << 0

    if i < len(font.offsets)-1:
        offsets += str(value) + ","
    else:
        offsets += str(value)

# Print font resource object
font_str = ""
font_str += "font_resource_class_obj_t font = {\n"
font_str += "  .base.type = &font_resource_class_type,\n"
font_str += "  .glyph_height = " + str(font.texture.height-1) + ",\n"
font_str += "  .glyph_widths = { " + widths + " },\n"
font_str += "  .glyph_x_offsets = { " + offsets + " },\n"
font_str += "  .glyph_widths_bytearray_ref = mp_const_none,\n"
font_str += "  .glyph_offsets_bytearray_ref = mp_const_none,\n"
font_str += "  .texture_resource = &font_texture\n"
font_str += "};\n"
print(font_str)