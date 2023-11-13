#include "draw/engine_display_draw.h"
#include "display/engine_display_common.h"
#include "debug/debug_print.h"

#include "math/vector3.h"
#include "math/rectangle.h"

#include <string.h>


inline bool is_xy_inside_screen(int32_t x, int32_t y){
    if(x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT){
        return true;
    }

    return false;
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