#include "engine_display_draw.h"
#include "engine_display_common.h"

#include <string.h>

void engine_draw_fill(uint16_t color){
    memset(engine_get_active_screen_buffer(), color, SCREEN_BUFFER_SIZE);
}


void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height){
    // if(x>=SCREEN_WIDTH || y>=SCREEN_HEIGHT){
    //     ENGINE_WARNING_PRINTF("Tried to blit of screen bounds");
    //     return;
    // }

    // uint16_t *screen_buffer = engine_get_active_screen_buffer();
    // uint16_t pixels_in_screen_buffer_start = (y * width) + x;

    // for(uint16_t ihx=0; ihx<height; ihx++){
    //     for(uint16_t width, )
    // }
}