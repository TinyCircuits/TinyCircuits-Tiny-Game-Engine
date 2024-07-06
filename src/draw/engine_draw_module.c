#include "py/obj.h"
#include "py/runtime.h"
#include "display/engine_display_common.h"
#include "resources/engine_texture_resource.h"
#include "resources/engine_resource_manager.h"
#include "engine_color.h"
#include "debug/debug_print.h"
#include "engine_main.h"


/*  --- doc ---
    NAME: set_background_color
    ID: set_background_color
    DESC: Sets the background (the data that clears the framebuffer) to a color
    PARAM: [type={ref_link:Color}|int]   [name=background_color]  [value=Color or int (RGB565)]
    RETURN: None
*/
static mp_obj_t engine_draw_set_background_color(mp_obj_t background_color){
    engine_display_set_fill_color(engine_color_class_color_value(background_color));
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_draw_set_background_color_obj, engine_draw_set_background_color);


/*  --- doc ---
    NAME: set_background
    ID: set_background
    DESC: Sets the background (the data that clears the framebuffer) to a {ref_link:TextureResource}
    PARAM: [type=object]   [name=background]  [value={ref_link:TextureResource}]
    RETURN: None
*/
static mp_obj_t engine_draw_set_background(mp_obj_t background){
    texture_resource_class_obj_t *background_texture_resource = background;

    if(background_texture_resource->width != SCREEN_WIDTH || background_texture_resource->height != SCREEN_HEIGHT){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Could not background image, images dimensions are not equal to screen dimensions!"));
    }

    uint16_t *texture_data = ((mp_obj_array_t*)background_texture_resource->data)->items;
    engine_display_set_fill_background(texture_data);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_draw_set_background_obj, engine_draw_set_background);


static mp_obj_t engine_draw_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_module_init_obj, engine_draw_module_init);


/*  --- doc ---
    NAME: back_fb_data
    ID: back_fb_data
    DESC: Get or set the back framebuffer data.
    PARAM: [type=bytearray (optional)]   [name=back_fb_data]  [value=bytearray 128*128*2]
    RETURN: bytearray | None
*/
static mp_obj_t engine_draw_back_fb_data(size_t n_args, const mp_obj_t *args){
    if (n_args == 0) {
        return MP_STATE_VM(back_fb_data);
    } else {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting back_fb_data not supported yet!"));
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_back_fb_data_obj, 0, 1, engine_draw_back_fb_data);

/*  --- doc ---
    NAME: front_fb_data
    ID: front_fb_data
    DESC: Get or set the front framebuffer data.
    PARAM: [type=bytearray (optional)]   [name=front_fb_data]  [value=bytearray 128*128*2]
    RETURN: bytearray | None
*/
static mp_obj_t engine_draw_front_fb_data(size_t n_args, const mp_obj_t *args){
    if (n_args == 0) {
        return MP_STATE_VM(front_fb_data);
    } else {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting front_fb_data not supported yet!"));
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_front_fb_data_obj, 0, 1, engine_draw_front_fb_data);

/*  --- doc ---
    NAME: back_fb
    ID: back_fb
    DESC: Get or set the back framebuffer.
    PARAM: [type=framebuf (optional)]   [name=back_fb]  [value=framebuf]
    RETURN: framebuf | None
*/
static mp_obj_t engine_draw_back_fb(size_t n_args, const mp_obj_t *args){
    if (n_args == 0) {
        return MP_STATE_VM(back_fb);
    } else {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting back_fb not supported yet!"));
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_back_fb_obj, 0, 1, engine_draw_back_fb);

/*  --- doc ---
    NAME: front_fb
    ID: front_fb
    DESC: Get or set the front framebuffer.
    PARAM: [type=framebuf (optional)]   [name=front_fb]  [value=framebuf]
    RETURN: framebuf | None
*/
static mp_obj_t engine_draw_front_fb(size_t n_args, const mp_obj_t *args){
    if (n_args == 0) {
        return MP_STATE_VM(front_fb);
    } else {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting front_fb not supported yet!"));
    }
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_front_fb_obj, 0, 1, engine_draw_front_fb);


color_class_obj_t black         = {{&const_color_class_type}, .value = 0x0000};
color_class_obj_t navy          = {{&const_color_class_type}, .value = 0x000F};
color_class_obj_t darkgreen     = {{&const_color_class_type}, .value = 0x03E0};
color_class_obj_t darkcyan      = {{&const_color_class_type}, .value = 0x03EF};
color_class_obj_t maroon        = {{&const_color_class_type}, .value = 0x7800};
color_class_obj_t purple        = {{&const_color_class_type}, .value = 0x780F};
color_class_obj_t olive         = {{&const_color_class_type}, .value = 0x7BE0};
color_class_obj_t lightgrey     = {{&const_color_class_type}, .value = 0xD69A};
color_class_obj_t darkgrey      = {{&const_color_class_type}, .value = 0x7BEF};
color_class_obj_t blue          = {{&const_color_class_type}, .value = 0x001F};
color_class_obj_t green         = {{&const_color_class_type}, .value = 0x07E0};
color_class_obj_t cyan          = {{&const_color_class_type}, .value = 0x07FF};
color_class_obj_t red           = {{&const_color_class_type}, .value = 0xF800};
color_class_obj_t magenta       = {{&const_color_class_type}, .value = 0xF81F};
color_class_obj_t yellow        = {{&const_color_class_type}, .value = 0xFFE0};
color_class_obj_t white         = {{&const_color_class_type}, .value = 0xFFFF};
color_class_obj_t orange        = {{&const_color_class_type}, .value = 0xFDA0};
color_class_obj_t greenyellow   = {{&const_color_class_type}, .value = 0xB7E0};
color_class_obj_t pink          = {{&const_color_class_type}, .value = 0xFE19};
color_class_obj_t brown         = {{&const_color_class_type}, .value = 0x9A60};
color_class_obj_t gold          = {{&const_color_class_type}, .value = 0xFEA0};
color_class_obj_t silver        = {{&const_color_class_type}, .value = 0xC618};
color_class_obj_t skyblue       = {{&const_color_class_type}, .value = 0x867D};
color_class_obj_t violet        = {{&const_color_class_type}, .value = 0x915C};


/*  --- doc ---
    NAME: engine_draw
    ID: engine_draw
    DESC: Module for drawing to the framebuffer
    ATTR: [type=function]           [name={ref_link:set_background_color}]  [value=function]
    ATTR: [type=function]           [name={ref_link:set_background}]        [value=function]
    ATTR: [type=function]           [name={ref_link:back_fb_data}]          [value=getter/setter function]
    ATTR: [type=function]           [name={ref_link:front_fb_data}]         [value=getter/setter function]
    ATTR: [type=function]           [name={ref_link:back_fb}]               [value=getter/setter function]
    ATTR: [type=function]           [name={ref_link:front_fb}]              [value=getter/setter function]
    ATTR: [type=type]               [name={ref_link:Color}]                 [value=type]
    ATTR: [type={ref_link:Color}]   [name=black]                            [value=0x0000]
    ATTR: [type={ref_link:Color}]   [name=navy]                             [value=0x000F]
    ATTR: [type={ref_link:Color}]   [name=darkgreen]                        [value=0x03E0]
    ATTR: [type={ref_link:Color}]   [name=darkcyan]                         [value=0x03EF]
    ATTR: [type={ref_link:Color}]   [name=maroon]                           [value=0x7800]
    ATTR: [type={ref_link:Color}]   [name=purple]                           [value=0x780F]
    ATTR: [type={ref_link:Color}]   [name=olive]                            [value=0x7BE0]
    ATTR: [type={ref_link:Color}]   [name=lightgrey]                        [value=0xD69A]
    ATTR: [type={ref_link:Color}]   [name=darkgrey]                         [value=0x7BEF]
    ATTR: [type={ref_link:Color}]   [name=blue]                             [value=0x001F]
    ATTR: [type={ref_link:Color}]   [name=green]                            [value=0x07E0]
    ATTR: [type={ref_link:Color}]   [name=cyan]                             [value=0x07FF]
    ATTR: [type={ref_link:Color}]   [name=red]                              [value=0xF800]
    ATTR: [type={ref_link:Color}]   [name=magenta]                          [value=0xF81F]
    ATTR: [type={ref_link:Color}]   [name=yellow]                           [value=0xFFE0]
    ATTR: [type={ref_link:Color}]   [name=white]                            [value=0xFFFF]
    ATTR: [type={ref_link:Color}]   [name=orange]                           [value=0xFDA0]
    ATTR: [type={ref_link:Color}]   [name=greenyellow]                      [value=0xB7E0]
    ATTR: [type={ref_link:Color}]   [name=pink]                             [value=0xFE19]
    ATTR: [type={ref_link:Color}]   [name=brown]                            [value=0x9A60]
    ATTR: [type={ref_link:Color}]   [name=gold]                             [value=0xFEA0]
    ATTR: [type={ref_link:Color}]   [name=silver]                           [value=0xC618]
    ATTR: [type={ref_link:Color}]   [name=skyblue]                          [value=0x867D]
    ATTR: [type={ref_link:Color}]   [name=violet]                           [value=0x915C]
*/
static const mp_rom_map_elem_t engine_draw_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_draw) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), MP_ROM_PTR(&engine_draw_module_init_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_background_color), MP_ROM_PTR(&engine_draw_set_background_color_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_set_background), MP_ROM_PTR(&engine_draw_set_background_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Color), MP_ROM_PTR(&color_class_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_black), MP_ROM_PTR(&black) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_navy), MP_ROM_PTR(&navy) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_darkgreen), MP_ROM_PTR(&darkgreen) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_darkcyan), MP_ROM_PTR(&darkcyan) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_maroon), MP_ROM_PTR(&maroon) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_purple), MP_ROM_PTR(&purple) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_olive), MP_ROM_PTR(&olive) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_lightgrey), MP_ROM_PTR(&lightgrey) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_darkgrey), MP_ROM_PTR(&darkgrey) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_blue), MP_ROM_PTR(&blue) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_green), MP_ROM_PTR(&green) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_cyan), MP_ROM_PTR(&cyan) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_red), MP_ROM_PTR(&red) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_magenta), MP_ROM_PTR(&magenta) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_yellow), MP_ROM_PTR(&yellow) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_white), MP_ROM_PTR(&white) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_orange), MP_ROM_PTR(&orange) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_greenyellow), MP_ROM_PTR(&greenyellow) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pink), MP_ROM_PTR(&pink) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_brown), MP_ROM_PTR(&brown) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_gold), MP_ROM_PTR(&gold) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_silver), MP_ROM_PTR(&silver) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_skyblue), MP_ROM_PTR(&skyblue) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_violet), MP_ROM_PTR(&violet) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_back_fb_data), MP_ROM_PTR(&engine_draw_back_fb_data_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_front_fb_data), MP_ROM_PTR(&engine_draw_front_fb_data_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_back_fb), MP_ROM_PTR(&engine_draw_back_fb_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_front_fb), MP_ROM_PTR(&engine_draw_front_fb_obj) },
};
static MP_DEFINE_CONST_DICT (mp_module_engine_draw_globals, engine_draw_globals_table);


const mp_obj_module_t engine_draw_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_draw_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_draw, engine_draw_user_cmodule);
