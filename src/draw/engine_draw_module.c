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
    PARAM: [type=enum/int]   [name=background_color]  [value=enum/int (16-bit RGB565)]
    RETURN: None
*/
static mp_obj_t engine_draw_set_background_color(mp_obj_t module, mp_obj_t background_color){
    color_class_obj_t *color = background_color;
    engine_display_set_fill_color(color->value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(engine_draw_set_background_color_obj, engine_draw_set_background_color);


/*  --- doc ---
    NAME: set_background
    ID: set_background
    DESC: Sets the background (the data that clears the framebuffer) to a {ref_link:TextureResource}
    PARAM: [type=object]   [name=background]  [value={ref_link:TextureResource}]
    RETURN: None
*/
static mp_obj_t engine_draw_set_background(mp_obj_t module, mp_obj_t background){
    texture_resource_class_obj_t *background_texture_resource = background;

    if(background_texture_resource->width != SCREEN_WIDTH || background_texture_resource->height != SCREEN_HEIGHT){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Could not background image, images dimensions are not equal to screen dimensions!"));
    }

    uint16_t *texture_data = ((mp_obj_array_t*)background_texture_resource->data)->items;
    engine_display_set_fill_background(texture_data);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(engine_draw_set_background_obj, engine_draw_set_background);


static mp_obj_t engine_draw_module_init(){
    engine_main_raise_if_not_initialized();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_module_init_obj, engine_draw_module_init);


/*  --- doc ---
    NAME: engine_draw
    ID: engine_draw
    DESC: Module for drawing to the framebuffer
    ATTR: [type=function]   [name={ref_link:set_background_color}]  [value=function]
    ATTR: [type=function]   [name={ref_link:set_background}]        [value=function]
    ATTR: [type=enum/int]   [name=black]                            [value=0x0000]
    ATTR: [type=enum/int]   [name=navy]                             [value=0x000F]
    ATTR: [type=enum/int]   [name=darkgreen]                        [value=0x03E0]
    ATTR: [type=enum/int]   [name=darkcyan]                         [value=0x03EF]
    ATTR: [type=enum/int]   [name=maroon]                           [value=0x7800]
    ATTR: [type=enum/int]   [name=purple]                           [value=0x780F]
    ATTR: [type=enum/int]   [name=olive]                            [value=0x7BE0]
    ATTR: [type=enum/int]   [name=lightgrey]                        [value=0xD69A]
    ATTR: [type=enum/int]   [name=darkgrey]                         [value=0x7BEF]
    ATTR: [type=enum/int]   [name=blue]                             [value=0x001F]
    ATTR: [type=enum/int]   [name=green]                            [value=0x07E0]
    ATTR: [type=enum/int]   [name=cyan]                             [value=0x07FF]
    ATTR: [type=enum/int]   [name=red]                              [value=0xF800]
    ATTR: [type=enum/int]   [name=magenta]                          [value=magenta]
    ATTR: [type=enum/int]   [name=yellow]                           [value=yellow]
    ATTR: [type=enum/int]   [name=white]                            [value=white]
    ATTR: [type=enum/int]   [name=orange]                           [value=orange]
    ATTR: [type=enum/int]   [name=greenyellow]                      [value=greenyellow]
    ATTR: [type=enum/int]   [name=pink]                             [value=0xFE19]
    ATTR: [type=enum/int]   [name=brown]                            [value=0x9A60]
    ATTR: [type=enum/int]   [name=gold]                             [value=0xFEA0]
    ATTR: [type=enum/int]   [name=silver]                           [value=0xC618]
    ATTR: [type=enum/int]   [name=skyblue]                          [value=0x867D]
    ATTR: [type=enum/int]   [name=violet]                           [value=0x915C]
    ATTR: [type=bytearray]  [name=back_fb_data]                     [value=128*128*2 bytearray]
    ATTR: [type=bytearray]  [name=front_fb_data]                    [value=128*128*2 bytearray]
    ATTR: [type=framebuf]   [name=back_fb]                          [value=framebuf]
    ATTR: [type=framebuf]   [name=front_fb]                         [value=framebuf]
*/
static const mp_rom_map_elem_t engine_draw_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_draw) },
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&engine_draw_module_init_obj }
};

// Module init
static MP_DEFINE_CONST_DICT (mp_module_engine_draw_globals, engine_draw_globals_table);


static void draw_class_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind){
    ENGINE_FORCE_PRINTF("print(): Draw");
}


static void draw_class_attr(mp_obj_t self_in, qstr attribute, mp_obj_t *destination){
    if(destination[0] == MP_OBJ_NULL){          // Load
        switch(attribute) {
            case MP_QSTR_set_background_color:
                destination[0] = MP_OBJ_FROM_PTR(&engine_draw_set_background_color_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_set_background:
                destination[0] = MP_OBJ_FROM_PTR(&engine_draw_set_background_obj);
                destination[1] = self_in;
            break;
            case MP_QSTR_Color:
                destination[0] = (mp_obj_type_t*)&color_class_type;
            break;
            case MP_QSTR_black:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x0000)});
            break;
            case MP_QSTR_navy:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x000F)});
            break;
            case MP_QSTR_darkgreen:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x03E0)});
            break;
            case MP_QSTR_darkcyan:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x03EF)});
            break;
            case MP_QSTR_maroon:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x7800)});
            break;
            case MP_QSTR_purple:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x780F)});
            break;
            case MP_QSTR_olive:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x7BE0)});
            break;
            case MP_QSTR_lightgrey:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xD69A)});
            break;
            case MP_QSTR_darkgrey:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x7BEF)});
            break;
            case MP_QSTR_blue:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x001F)});
            break;
            case MP_QSTR_green:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x07E0)});
            break;
            case MP_QSTR_cyan:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x07FF)});
            break;
            case MP_QSTR_red:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xF800)});
            break;
            case MP_QSTR_magenta:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xF81F)});
            break;
            case MP_QSTR_yellow:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xFFE0)});
            break;
            case MP_QSTR_white:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xFFFF)});
            break;
            case MP_QSTR_orange:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xFDA0)});
            break;
            case MP_QSTR_greenyellow:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xB7E0)});
            break;
            case MP_QSTR_pink:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xFE19)});
            break;
            case MP_QSTR_brown:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x9A60)});
            break;
            case MP_QSTR_gold:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xFEA0)});
            break;
            case MP_QSTR_silver:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0xC618)});
            break;
            case MP_QSTR_skyblue:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x867D)});
            break;
            case MP_QSTR_violet:
                destination[0] = color_class_new(&color_class_type, 1, 0, (mp_obj_t[]){mp_obj_new_int(0x915C)});
            break;
            case MP_QSTR_back_fb_data:
                destination[0] = MP_STATE_VM(back_fb_data);
            break;
            case MP_QSTR_front_fb_data:
                destination[0] = MP_STATE_VM(front_fb_data);
            break;
            case MP_QSTR_back_fb:
                destination[0] = MP_STATE_VM(back_fb);
            break;
            case MP_QSTR_front_fb:
                destination[0] = MP_STATE_VM(front_fb);
            break;
            default:
                return; // Fail
        }
    }else if(destination[1] != MP_OBJ_NULL){    // Store
        switch(attribute) {
            case MP_QSTR_back_fb_data:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting the back buffer data is not supported yet!"));
            break;
            case MP_QSTR_front_fb_data:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting the back buffer is not supported yet!"));
            break;
            case MP_QSTR_back_fb:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting the front buffer data is not supported yet!"));
            break;
            case MP_QSTR_front_fb:
                mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ERROR: Setting the front buffer is not supported yet!"));
            break;
            default:
                return; // Fail
        }
    }
}


MP_DEFINE_CONST_OBJ_TYPE(
    mp_type_color_module,
    MP_QSTR_module,
    MP_TYPE_FLAG_NONE,
    print, draw_class_print,
    attr, draw_class_attr
);

const mp_obj_module_t engine_draw_user_cmodule = {
    .base = { &mp_type_color_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_draw_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_draw, engine_draw_user_cmodule);