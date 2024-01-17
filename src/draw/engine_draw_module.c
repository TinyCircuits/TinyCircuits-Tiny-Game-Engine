#include "py/obj.h"
#include "display/engine_display_common.h"
#include "resources/engine_texture_resource.h"


STATIC mp_obj_t engine_draw_set_background_color(mp_obj_t background_color){
    engine_fill_color = mp_obj_get_int(background_color);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_draw_set_background_color_obj, engine_draw_set_background_color);


STATIC mp_obj_t engine_draw_set_background(mp_obj_t background){
    texture_resource_class_obj_t *background_texture_resource = background;

    if(background_texture_resource->width != SCREEN_WIDTH || background_texture_resource->height != SCREEN_HEIGHT){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Could not background image, images dimensions are not equal to screen dimensions!"));
    }

    engine_fill_background = background_texture_resource->data;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_draw_set_background_obj, engine_draw_set_background);


// Module attributes
STATIC const mp_rom_map_elem_t engine_draw_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_draw) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_background_color), (mp_obj_t)&engine_draw_set_background_color_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_background), (mp_obj_t)&engine_draw_set_background_obj },

    // // https://github.com/Bodmer/TFT_eSPI/blob/cbf06d7a214938d884b21d5aeb465241c25ce774/TFT_eSPI.h#L304-L328
    { MP_ROM_QSTR(MP_QSTR_black), MP_ROM_INT(0x0000) },
    { MP_ROM_QSTR(MP_QSTR_navy), MP_ROM_INT(0x000F) },
    { MP_ROM_QSTR(MP_QSTR_darkgreen), MP_ROM_INT(0x03E0) },
    { MP_ROM_QSTR(MP_QSTR_darkcyan), MP_ROM_INT(0x03EF) },
    { MP_ROM_QSTR(MP_QSTR_maroon), MP_ROM_INT(0x7800) },
    { MP_ROM_QSTR(MP_QSTR_purple), MP_ROM_INT(0x780F) },
    { MP_ROM_QSTR(MP_QSTR_olive), MP_ROM_INT(0x7BE0) },
    { MP_ROM_QSTR(MP_QSTR_lightgrey), MP_ROM_INT(0xD69A) },
    { MP_ROM_QSTR(MP_QSTR_darkgrey), MP_ROM_INT(0x7BEF) },
    { MP_ROM_QSTR(MP_QSTR_blue), MP_ROM_INT(0x001F) },
    { MP_ROM_QSTR(MP_QSTR_green), MP_ROM_INT(0x07E0) },
    { MP_ROM_QSTR(MP_QSTR_cyan), MP_ROM_INT(0x07FF) },
    { MP_ROM_QSTR(MP_QSTR_red), MP_ROM_INT(0xF800) },
    { MP_ROM_QSTR(MP_QSTR_magenta), MP_ROM_INT(0xF81F) },
    { MP_ROM_QSTR(MP_QSTR_yellow), MP_ROM_INT(0xFFE0) },
    { MP_ROM_QSTR(MP_QSTR_white), MP_ROM_INT(0xFFFF) },
    { MP_ROM_QSTR(MP_QSTR_orange), MP_ROM_INT(0xFDA0) },
    { MP_ROM_QSTR(MP_QSTR_greenyellow), MP_ROM_INT(0xB7E0) },
    { MP_ROM_QSTR(MP_QSTR_pink), MP_ROM_INT(0xFE19) },
    { MP_ROM_QSTR(MP_QSTR_brown), MP_ROM_INT(0x9A60) },
    { MP_ROM_QSTR(MP_QSTR_gold), MP_ROM_INT(0xFEA0) },
    { MP_ROM_QSTR(MP_QSTR_silver), MP_ROM_INT(0xC618) },
    { MP_ROM_QSTR(MP_QSTR_skyblue), MP_ROM_INT(0x867D) },
    { MP_ROM_QSTR(MP_QSTR_violet), MP_ROM_INT(0x915C) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_draw_globals, engine_draw_globals_table);

const mp_obj_module_t engine_draw_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_draw_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_draw, engine_draw_user_cmodule);