#include "engine_display_draw.h"
#include "engine_display_common.h"

#include "../math/vector3.h"
#include "../math/rectangle.h"

#include <string.h>


inline bool is_xy_inside_viewport(int32_t x, int32_t y, rectangle_class_obj_t *camera_viewport){
    if( x >= (int32_t)camera_viewport->x &&
        y >= (int32_t)camera_viewport->y &&
        x < (int32_t)camera_viewport->x+camera_viewport->width &&
        y < (int32_t)camera_viewport->y+camera_viewport->height){

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


void engine_draw_pixel(uint16_t color, int32_t x, int32_t y, engine_camera_node_class_obj_t *camera){
    uint16_t *screen_buffer = engine_get_active_screen_buffer();

    if(camera == NULL){
        if(is_xy_inside_screen(x, y)){
            screen_buffer[y*SCREEN_WIDTH + x] = color;
        }else{
            ENGINE_WARNING_PRINTF("Tried to draw pixel outside of screen bounds, clipped");
        }
    }else{
        vector3_class_obj_t *camera_position = camera->position;
        rectangle_class_obj_t *camera_viewport = camera->viewport;

        int32_t result_x = ((int32_t)camera_viewport->x) + (x - (int32_t)camera_position->x);
        int32_t result_y = ((int32_t)camera_viewport->y) + (y - (int32_t)camera_position->y);

        if(is_xy_inside_viewport(result_x, result_y, camera_viewport)){
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