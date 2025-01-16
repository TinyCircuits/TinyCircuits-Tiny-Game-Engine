#include "py/obj.h"
#include "py/runtime.h"
#include "display/engine_display_common.h"
#include "resources/engine_texture_resource.h"
#include "resources/engine_font_resource.h"
#include "resources/engine_resource_manager.h"
#include "engine_color.h"
#include "debug/debug_print.h"
#include "engine_main.h"
#include "engine_display_draw.h"
#include "math/engine_math.h"
#include "math/vector2.h"
#include "display/engine_display.h"

#include <string.h>


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

    // Needs to be the same size
    if(background_texture_resource->width != SCREEN_WIDTH || background_texture_resource->height != SCREEN_HEIGHT){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineDraw: ERROR: Could not set background bitmap, bitmap dimensions are not equal to screen dimensions!"));
    }

    // Needs to be RGB565
    if(background_texture_resource->red_mask   != 0b1111100000000000 ||
       background_texture_resource->green_mask != 0b0000011111100000 ||
       background_texture_resource->blue_mask  != 0b0000000000011111){
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineDraw: ERROR: Could not set background bitmap, bitmap needs to be RGB565 format!"));
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
    DESC: Get the mutable back framebuffer data (this is the buffer that the engine draws to).
    RETURN: bytearray
*/
static mp_obj_t engine_draw_back_fb_data(){
    return MP_STATE_VM(back_fb_data);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_back_fb_data_obj, engine_draw_back_fb_data);

/*  --- doc ---
    NAME: front_fb_data
    ID: front_fb_data
    DESC: Get the mutable front framebuffer data (shouldn't ever need to access this, this is the buffer actively being sent to the screen).
    RETURN: bytearray
*/
static mp_obj_t engine_draw_front_fb_data(){
    return MP_STATE_VM(front_fb_data);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_front_fb_data_obj, engine_draw_front_fb_data);

/*  --- doc ---
    NAME: back_fb
    ID: back_fb
    DESC: Get the back framebuffer which can be directly manipulated (this is the buffer that the engine draws to).
    RETURN: framebuf
*/
static mp_obj_t engine_draw_back_fb(){
    return MP_STATE_VM(back_fb);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_back_fb_obj, engine_draw_back_fb);

/*  --- doc ---
    NAME: front_fb
    ID: front_fb
    DESC: Get the front framebuffer which can be directly manipulated (shouldn't ever need to access this, this is the buffer actively being sent to the screen).
    RETURN: framebuf
*/
static mp_obj_t engine_draw_front_fb(){
    return MP_STATE_VM(front_fb);
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_front_fb_obj, engine_draw_front_fb);


/*  --- doc ---
    NAME: clear
    ID: engine_draw_module_clear
    DESC: Clears the current screen buffer to the passed color.
    PARAM: [type=uint16 | {ref_link:Color}]  [name=color]    [value=positive unsigned RGB565 16-bit int or {ref_link:Color}]
    RETURN: None
*/
static mp_obj_t engine_draw_module_clear(mp_obj_t arg) {
    uint16_t color = engine_color_class_color_value(arg);
    engine_display_clear_screen_buffer(color);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(engine_draw_module_clear_obj, engine_draw_module_clear);


/*  --- doc ---
    NAME: pixel
    ID: engine_draw_module_pixel
    DESC: Directly draws a pixel to the screen without camera transformation. All parameters are required and keywords are not allowed. This keeps this function as fast as possible.
    PARAM: [type=uint16 | {ref_link:Color}]  [name=color]    [value=positive unsigned RGB565 16-bit int or {ref_link:Color}]
    PARAM: [type=float]                      [name=x]        [value=any]
    PARAM: [type=float]                      [name=y]        [value=any]
    PARAM: [type=float]                      [name=opacity]  [value=0.0 ~ 1.0]
    RETURN: None
*/
static mp_obj_t engine_draw_module_pixel(size_t n_args, const mp_obj_t *args) {    
    uint16_t color = engine_color_class_color_value(args[0]);
    float x  = mp_obj_get_float(args[1]);
    float y  = mp_obj_get_float(args[2]);
    float opacity  = mp_obj_get_float(args[3]);
    
    engine_shader_t *shader = NULL;
    if(opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    engine_draw_pixel(color, (int32_t)x, (int32_t)y, opacity, shader);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_module_pixel_obj, 4, 4, engine_draw_module_pixel);


/*  --- doc ---
    NAME: line
    ID: engine_draw_module_line
    DESC: Directly draws a line to the screen without camera transformation. All parameters are required and keywords are not allowed. This keeps this function as fast as possible.
    PARAM: [type=uint16 | {ref_link:Color}]  [name=color]    [value=positive unsigned RGB565 16-bit int or {ref_link:Color}]
    PARAM: [type=float]                      [name=start_x]  [value=any]
    PARAM: [type=float]                      [name=start_y]  [value=any]
    PARAM: [type=float]                      [name=end_x]    [value=any]
    PARAM: [type=float]                      [name=end_y]    [value=any]
    PARAM: [type=float]                      [name=opacity]  [value=0.0 ~ 1.0]
    RETURN: None
*/
static mp_obj_t engine_draw_module_line(size_t n_args, const mp_obj_t *args) {    
    uint16_t color = engine_color_class_color_value(args[0]);
    float start_x  = mp_obj_get_float(args[1]);
    float start_y  = mp_obj_get_float(args[2]);
    float end_x    = mp_obj_get_float(args[3]);
    float end_y    = mp_obj_get_float(args[4]);
    float opacity  = mp_obj_get_float(args[5]);

    engine_shader_t *shader = NULL;
    if(opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }
    
    engine_draw_line(color, start_x, start_y, end_x, end_y, opacity, shader);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_module_line_obj, 6, 6, engine_draw_module_line);


/*  --- doc ---
    NAME: rect
    ID: engine_draw_module_rect
    DESC: Directly draws a rectangle to the screen without camera transformation. All parameters are required and keywords are not allowed. This keeps this function as fast as possible.
    PARAM: [type=uint16 | {ref_link:Color}]  [name=color]       [value=positive unsigned RGB565 16-bit int or {ref_link:Color}]
    PARAM: [type=float]                      [name=top_left_x]  [value=any]
    PARAM: [type=float]                      [name=top_left_y]  [value=any]
    PARAM: [type=float]                      [name=width]       [value=any]
    PARAM: [type=float]                      [name=height]      [value=any]
    PARAM: [type=boolean]                    [name=outline]     [value=any]
    PARAM: [type=float]                      [name=opacity]     [value=0.0 ~ 1.0]
    RETURN: None
*/
static mp_obj_t engine_draw_module_rect(size_t n_args, const mp_obj_t *args) {    
    uint16_t color    = engine_color_class_color_value(args[0]);
    float top_left_x  = mp_obj_get_float(args[1]);
    float top_left_y  = mp_obj_get_float(args[2]);
    float width       = mp_obj_get_float(args[3]);
    float height      = mp_obj_get_float(args[4]);
    bool outline      = mp_obj_get_int(args[5]);
    float opacity     = mp_obj_get_float(args[6]);
    
    float center_x = top_left_x + width * 0.5f;
    float center_y = top_left_y + height * 0.5f;

    engine_shader_t *shader = NULL;
    if(opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }
    
    if(outline){
        // Calculate the coordinates of the 4 corners of the rectangle, not rotated
        // NOTE: positive y is down
        float tlx = top_left_x;
        float tly = top_left_y;

        float trx = tlx+width;
        float try = tly;

        float brx = tlx+width;
        float bry = tly+height;

        float blx = tlx;
        float bly = tly+height;

        engine_draw_line(color, tlx, tly, trx, try, opacity, shader);
        engine_draw_line(color, trx, try, brx, bry, opacity, shader);
        engine_draw_line(color, brx, bry, blx, bly, opacity, shader);
        engine_draw_line(color, blx, bly, tlx, tly, opacity, shader);
    }else{
        engine_draw_rect(color, center_x, center_y, (int32_t)width, (int32_t)height, 1.0f, 1.0f, 0.0f, opacity, shader);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_module_rect_obj, 7, 7, engine_draw_module_rect);


/*  --- doc ---
    NAME: circle
    ID: engine_draw_module_circle
    DESC: Directly draws a circle to the screen without camera transformation. All parameters are required and keywords are not allowed. This keeps this function as fast as possible.
    PARAM: [type=uint16 | {ref_link:Color}]  [name=color]     [value=positive unsigned RGB565 16-bit int or {ref_link:Color}]
    PARAM: [type=float]                      [name=center_x]  [value=any]
    PARAM: [type=float]                      [name=center_y]  [value=any]
    PARAM: [type=float]                      [name=radius]    [value=any]
    PARAM: [type=boolean]                    [name=outline]   [value=any]
    PARAM: [type=float]                      [name=opacity]   [value=0.0 ~ 1.0]
    RETURN: None
*/
static mp_obj_t engine_draw_module_circle(size_t n_args, const mp_obj_t *args) {    
    uint16_t color  = engine_color_class_color_value(args[0]);
    float center_x  = mp_obj_get_float(args[1]);
    float center_y  = mp_obj_get_float(args[2]);
    float radius    = mp_obj_get_float(args[3]);
    bool outline    = mp_obj_get_int(args[4]);
    float opacity   = mp_obj_get_float(args[5]);

    engine_shader_t *shader = NULL;
    if(opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }
    
    if(outline){
        engine_draw_outline_circle(color, center_x, center_y, radius, opacity, shader);
    }else{
        engine_draw_filled_circle(color, center_x, center_y, radius, opacity, shader);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_module_circle_obj, 6, 6, engine_draw_module_circle);


/*  --- doc ---
    NAME: text
    ID: engine_draw_module_text
    DESC: Directly draws a text to the screen without camera transformation. All parameters are required and keywords are not allowed. This keeps this function as fast as possible.
    PARAM: [type={ref_link:FontResource} | `None`]    [name=font]            [value={ref_link:FontResource} | `None`]
    PARAM: [type=string]                              [name=text]            [value=any]
    PARAM: [type=uint16 | {ref_link:Color} | `None`]  [name=blend_color]     [value=positive unsigned RGB565 16-bit int | {ref_link:Color} | `None`]
    PARAM: [type=float]                               [name=top_left_x]      [value=any]
    PARAM: [type=float]                               [name=top_left_y]      [value=any]
    PARAM: [type=float]                               [name=letter_spacing]  [value=any]
    PARAM: [type=float]                               [name=line_spacing]    [value=any]
    PARAM: [type=float]                               [name=opacity]         [value=0.0 ~ 1.0]
    RETURN: None
*/
static mp_obj_t engine_draw_module_text(size_t n_args, const mp_obj_t *args) {
    font_resource_class_obj_t *font = args[0];
    mp_obj_t text                   = args[1];
    mp_obj_t blend_color            = args[2];
    float top_left_x                = mp_obj_get_float(args[3]);
    float top_left_y                = mp_obj_get_float(args[4]);
    float letter_spacing            = mp_obj_get_float(args[5]);
    float line_spacing              = mp_obj_get_float(args[6]);
    float opacity                   = mp_obj_get_float(args[7]);

    engine_shader_t *shader = NULL;

    if(blend_color != mp_const_none){
        shader = engine_get_builtin_shader(BLEND_OPACITY_SHADER);

        float t = 1.0f;

        uint16_t color = engine_color_class_color_value(blend_color);
        shader->program[1] = (color >> 8) & 0b11111111;
        shader->program[2] = (color >> 0) & 0b11111111;

        memcpy(shader->program+3, &t, sizeof(float));
    }else if(opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);   
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    if(font == mp_const_none){
        font = &default_font;
    }

    // Figure out dimensions of text and get center for drawing
    float width = 0.0f;
    float height = 0.0f;
    font_resource_get_box_dimensions(font, text, &width, &height, letter_spacing, line_spacing);
    float center_x = top_left_x + width * 0.5f;
    float center_y = top_left_y + height * 0.5f;

    engine_draw_text(font, text, center_x, center_y, width, height, letter_spacing, line_spacing, 1.0f, 1.0f, 0.0f, opacity, shader);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_module_text_obj, 8, 8, engine_draw_module_text);


/*  --- doc ---
    NAME: blit
    ID: engine_draw_module_blit
    DESC: Directly draws a texture to the screen without camera transformation. All parameters are required and keywords are not allowed. This keeps this function as fast as possible.
    PARAM: [type={ref_link:TextureResource}]    [name=texture]         [value={ref_link:TextureResource}]
    PARAM: [type=float]                         [name=top_left_x]      [value=any]
    PARAM: [type=float]                         [name=top_left_y]      [value=any]
    PARAM: [type=float]                         [name=opacity]         [value=any]
    RETURN: None
*/
static mp_obj_t engine_draw_module_blit(size_t n_args, const mp_obj_t *args) {
    texture_resource_class_obj_t *texture = args[0];
    float top_left_x                      = mp_obj_get_float(args[1]);
    float top_left_y                      = mp_obj_get_float(args[2]);
    float opacity                         = mp_obj_get_float(args[3]);

    engine_shader_t *shader = NULL;
    if(opacity < 1.0f){
        shader = engine_get_builtin_shader(OPACITY_SHADER);
    }else{
        shader = engine_get_builtin_shader(EMPTY_SHADER);
    }

    float width = texture->width;
    float height = texture->height;

    float center_x = top_left_x + width * 0.5f;
    float center_y = top_left_y + height * 0.5f;

    engine_draw_blit(texture, 0, center_x, center_y, (int32_t)width, (int32_t)height, (int32_t)width, 1.0f, 1.0f, 0.0f, ENGINE_NO_TRANSPARENCY_COLOR, opacity, shader);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(engine_draw_module_blit_obj, 4, 4, engine_draw_module_blit);


/*  --- doc ---
    NAME: update
    ID: engine_draw_module_update
    DESC: Sends the screen buffer to the screen and switches the internal dual buffers. Call this to update the screen after you have drawn everything you want.
    RETURN: None
*/
static mp_obj_t engine_draw_module_update() {
    engine_display_send();
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(engine_draw_module_update_obj, engine_draw_module_update);


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
    ATTR: [type=function]           [name={ref_link:set_background_color}]      [value=function]
    ATTR: [type=function]           [name={ref_link:set_background}]            [value=function]
    ATTR: [type=function]           [name={ref_link:back_fb_data}]              [value=getter/setter function]
    ATTR: [type=function]           [name={ref_link:front_fb_data}]             [value=getter/setter function]
    ATTR: [type=function]           [name={ref_link:back_fb}]                   [value=getter/setter function]
    ATTR: [type=function]           [name={ref_link:front_fb}]                  [value=getter/setter function]
    ATTR: [type=type]               [name={ref_link:Color}]                     [value=type]
    ATTR: [type={ref_link:Color}]   [name=black]                                [value=0x0000]
    ATTR: [type={ref_link:Color}]   [name=navy]                                 [value=0x000F]
    ATTR: [type={ref_link:Color}]   [name=darkgreen]                            [value=0x03E0]
    ATTR: [type={ref_link:Color}]   [name=darkcyan]                             [value=0x03EF]
    ATTR: [type={ref_link:Color}]   [name=maroon]                               [value=0x7800]
    ATTR: [type={ref_link:Color}]   [name=purple]                               [value=0x780F]
    ATTR: [type={ref_link:Color}]   [name=olive]                                [value=0x7BE0]
    ATTR: [type={ref_link:Color}]   [name=lightgrey]                            [value=0xD69A]
    ATTR: [type={ref_link:Color}]   [name=darkgrey]                             [value=0x7BEF]
    ATTR: [type={ref_link:Color}]   [name=blue]                                 [value=0x001F]
    ATTR: [type={ref_link:Color}]   [name=green]                                [value=0x07E0]
    ATTR: [type={ref_link:Color}]   [name=cyan]                                 [value=0x07FF]
    ATTR: [type={ref_link:Color}]   [name=red]                                  [value=0xF800]
    ATTR: [type={ref_link:Color}]   [name=magenta]                              [value=0xF81F]
    ATTR: [type={ref_link:Color}]   [name=yellow]                               [value=0xFFE0]
    ATTR: [type={ref_link:Color}]   [name=white]                                [value=0xFFFF]
    ATTR: [type={ref_link:Color}]   [name=orange]                               [value=0xFDA0]
    ATTR: [type={ref_link:Color}]   [name=greenyellow]                          [value=0xB7E0]
    ATTR: [type={ref_link:Color}]   [name=pink]                                 [value=0xFE19]
    ATTR: [type={ref_link:Color}]   [name=brown]                                [value=0x9A60]
    ATTR: [type={ref_link:Color}]   [name=gold]                                 [value=0xFEA0]
    ATTR: [type={ref_link:Color}]   [name=silver]                               [value=0xC618]
    ATTR: [type={ref_link:Color}]   [name=skyblue]                              [value=0x867D]
    ATTR: [type={ref_link:Color}]   [name=violet]                               [value=0x915C]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_clear}]  [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_pixel}]  [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_line}]   [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_rect}]   [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_circle}] [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_text}]   [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_blit}]   [value=function]
    ATTR: [type=function]           [name={ref_link:engine_draw_module_update}] [value=function]
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

    { MP_OBJ_NEW_QSTR(MP_QSTR_clear), MP_ROM_PTR(&engine_draw_module_clear_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&engine_draw_module_pixel_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_line), MP_ROM_PTR(&engine_draw_module_line_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_rect), MP_ROM_PTR(&engine_draw_module_rect_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_circle), MP_ROM_PTR(&engine_draw_module_circle_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_text), MP_ROM_PTR(&engine_draw_module_text_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_blit), MP_ROM_PTR(&engine_draw_module_blit_obj) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_update), MP_ROM_PTR(&engine_draw_module_update_obj) },
};
static MP_DEFINE_CONST_DICT (mp_module_engine_draw_globals, engine_draw_globals_table);


const mp_obj_module_t engine_draw_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_draw_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_draw, engine_draw_user_cmodule);
