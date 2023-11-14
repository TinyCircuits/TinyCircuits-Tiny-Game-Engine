#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"

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


void engine_draw_pixel_viewport(uint16_t color, int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh, int32_t px, int32_t py){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    int32_t result_x = vx + (x - px);
    int32_t result_y = vy + (y - py);

    if(is_xy_inside_viewport(result_x, result_y, vx, vy, vw, vh)){
        screen_buffer[result_y*SCREEN_WIDTH + result_x] = color;
    }else{
        ENGINE_WARNING_PRINTF("Tried to draw pixel outside of viewport bounds, clipped");
    }
}


void engine_draw_pixel(uint16_t color, int32_t x, int32_t y){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    if(is_xy_inside_screen(x, y)){
        screen_buffer[y*SCREEN_WIDTH + x] = color;
    }else{
        ENGINE_WARNING_PRINTF("Tried to draw pixel outside of screen bounds, clipped");
    }
}


void engine_draw_fill_screen_buffer(uint16_t color, uint16_t *screen_buffer){
    memset(screen_buffer, color, SCREEN_BUFFER_SIZE_BYTES);
}


void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height){
    
}