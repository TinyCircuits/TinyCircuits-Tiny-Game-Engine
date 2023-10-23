#include "engine_display_common.h"
#include <stdbool.h>
#include <stdlib.h>

// Two screen buffers are allocated so that one can be
// getting sent to the screen using the independent DMA
// hardware while the other is being used by the CPU
// to draw the next frame
static uint16_t **dual_screen_buffers;

// The current index of the 'active_screen_buffer' in 'dual_screen_buffers'
// (gets switched when the screen buffer is sent out over DMA)
static uint8_t active_screen_buffer_index = 0;

// The current screen buffer that should be getting drawn to (the other
// one is likely being sent to the screen while this is active)
static uint16_t *active_screen_buffer;

static bool is_initialzed = false;


void engine_init_screen_buffers(){
    if(is_initialzed == false){
        dual_screen_buffers = (uint16_t**)malloc(sizeof(uint16_t*) * 2);
        dual_screen_buffers[0] = (uint16_t*)malloc(SCREEN_BUFFER_SIZE);
        dual_screen_buffers[1] = (uint16_t*)malloc(SCREEN_BUFFER_SIZE);
        active_screen_buffer = dual_screen_buffers[0];
        is_initialzed = true;
    }
}


uint16_t *engine_get_active_screen_buffer(){
    return active_screen_buffer;
}


void engine_switch_active_screen_buffer(){
    // Switch which screen buffer should be drawn to
    if(active_screen_buffer_index == 0){
        active_screen_buffer_index = 1;
    }else{
        active_screen_buffer_index = 0;
    }

    active_screen_buffer = dual_screen_buffers[active_screen_buffer_index];
}