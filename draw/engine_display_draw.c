#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"

#include "math/vector3.h"
#include "math/rectangle.h"

#include <string.h>


inline bool is_xy_inside_viewport(int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh){
    if(x >= vx && y >= vy && x < vx+vw && y < vy+vh){
        return true;
    }

    return false;
}


inline bool is_xy_inside_screen(int32_t x, int32_t y){
    if(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT){
        return true;
    }

    return false;
}


void engine_draw_pixel(uint16_t color, int32_t x, int32_t y, mp_obj_t camera){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    if(camera == NULL){
        if(is_xy_inside_screen(x, y)){
            screen_buffer[y*SCREEN_WIDTH + x] = color;
        }else{
            ENGINE_WARNING_PRINTF("Tried to draw pixel outside of screen bounds, clipped");
        }
    }else{
        vector3_class_obj_t *camera_position = mp_load_attr(camera, MP_QSTR_position);
        // vector3_class_obj_t *camera_rotation = mp_load_attr(camera_node, MP_QSTR_rotation);
        rectangle_class_obj_t *camera_viewport = mp_load_attr(camera, MP_QSTR_viewport);

        int32_t vx = (int32_t)mp_obj_get_float(camera_viewport->x);
        int32_t vy = (int32_t)mp_obj_get_float(camera_viewport->y);
        uint8_t vw = (uint8_t)mp_obj_get_float(camera_viewport->width);
        uint8_t vh = (uint8_t)mp_obj_get_float(camera_viewport->height);

        int32_t result_x = ((int32_t)vx) + (x - (int32_t)camera_position->x);
        int32_t result_y = ((int32_t)vy) + (y - (int32_t)camera_position->y);

        if(is_xy_inside_viewport(result_x, result_y, vx, vy, vw, vh)){
            screen_buffer[result_y*SCREEN_WIDTH + result_x] = color;
        }else{
            ENGINE_WARNING_PRINTF("Tried to draw pixel outside of viewport bounds, clipped");
        }
    }
}


void engine_draw_fill_screen_buffer(uint16_t color, uint16_t *screen_buffer){
    memset(screen_buffer, color, SCREEN_BUFFER_SIZE_BYTES);
}


void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height){
    
}