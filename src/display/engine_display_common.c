#include "engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "debug/debug_print.h"
#include "utility/engine_defines.h"
#include "utility/engine_mp.h"
#include "py/obj.h"
#include "py/misc.h"
#include <stdlib.h>

uint16_t engine_fill_color = 0x0000;
uint16_t *engine_fill_background = NULL;

// Two screen buffers are allocated so that one can be
// getting sent to the screen using the independent DMA
// hardware while the other is being used by the CPU
// to draw the next frame
static uint16_t **dual_screen_buffers;

// The current index of the 'active_screen_buffer' in 'dual_screen_buffers'
// (gets switched when the screen buffer is sent out over DMA)
static uint8_t active_screen_buffer_index = 0;

// Certain nodes need a way of tracking the depth of
// their pixels for occlusion. If a node that requires
// that is spawned, it will ensure this buffer is
// generated only at the point
uint16_t *depth_buffer = NULL;


void engine_display_set_fill_color(uint16_t color){
    engine_fill_color = color;
}


void engine_display_set_fill_background(uint16_t *data){
    engine_fill_background = data;
}


void engine_init_screen_buffers(){
    ENGINE_INFO_PRINTF("Creating dual screen buffers of size %d bytes each on C heap", SCREEN_BUFFER_SIZE_BYTES);
    dual_screen_buffers = (uint16_t**)m_tracked_calloc(1, sizeof(uint16_t*) * 2);
    dual_screen_buffers[0] = (uint16_t*)m_tracked_calloc(1, SCREEN_BUFFER_SIZE_BYTES);
    dual_screen_buffers[1] = (uint16_t*)m_tracked_calloc(1, SCREEN_BUFFER_SIZE_BYTES);

    // Make sure both screen buffers are set to all zeros
    ENGINE_INFO_PRINTF("Filling both screen buffers with 0x0");
    engine_draw_fill_color(0x0, dual_screen_buffers[0]);
    engine_draw_fill_color(0x0, dual_screen_buffers[1]);

    active_screen_buffer = dual_screen_buffers[0];
}


void engine_switch_active_screen_buffer(){
    // Switch which screen buffer should be drawn to
    active_screen_buffer_index = 1 - active_screen_buffer_index;

    active_screen_buffer = dual_screen_buffers[active_screen_buffer_index];
}


void ENGINE_FAST_FUNCTION(engine_display_clear_depth_buffer)(){
    if(depth_buffer != NULL) engine_draw_fill_color(UINT16_MAX, depth_buffer);
}


void engine_display_check_depth_buffer_created(){
    if(depth_buffer == NULL){
        depth_buffer = m_tracked_calloc(2, SCREEN_WIDTH*SCREEN_HEIGHT);
        engine_display_clear_depth_buffer();
    }
}


void engine_display_free_depth_buffer(){
    if(depth_buffer != NULL){
        m_tracked_free(depth_buffer);
        depth_buffer = NULL;
    }
}


bool ENGINE_FAST_FUNCTION(engine_display_store_check_depth_index)(uint16_t index, uint16_t depth){
    if(depth < depth_buffer[index]){
        depth_buffer[index] = depth;
        return true;
    }

    return false;
}


bool ENGINE_FAST_FUNCTION(engine_display_store_check_depth)(uint8_t sx, uint8_t sy, uint16_t depth){
    uint16_t index = sy * SCREEN_WIDTH + sx;
    return engine_display_store_check_depth_index(index, depth);
}