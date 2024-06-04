#include "engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "debug/debug_print.h"
#include "utility/engine_defines.h"
#include "utility/engine_mp.h"
#include "resources/engine_resource_manager.h"
#include "py/misc.h"
#include <stdlib.h>
#include "py/objarray.h"

uint16_t engine_fill_color = 0x0000;
uint16_t *engine_fill_background = NULL;

// The current index of the 'active_screen_buffer' in 'dual_screen_buffers'
// (gets switched when the screen buffer is sent out over DMA)
static uint8_t active_screen_buffer_index = 0;


void engine_display_set_fill_color(uint16_t color){
    engine_fill_color = color;
}


void engine_display_set_fill_background(uint16_t *data){
    engine_fill_background = data;
}


void engine_init_screen_buffers(){
    ENGINE_INFO_PRINTF("Creating dual screen buffers of size %d bytes each on C heap", SCREEN_BUFFER_SIZE_BYTES);

    // screen_buffers[0] = m_tracked_calloc(1, SCREEN_BUFFER_SIZE_BYTES);
    // screen_buffers[1] = m_tracked_calloc(1, SCREEN_BUFFER_SIZE_BYTES);

    screen_buffers[0] = malloc(SCREEN_BUFFER_SIZE_BYTES);
    screen_buffers[1] = malloc(SCREEN_BUFFER_SIZE_BYTES);

    // Make sure both screen buffers are set to all zeros
    ENGINE_INFO_PRINTF("Filling both screen buffers with 0x0");
    engine_draw_fill_color(0x0, screen_buffers[0]);
    engine_draw_fill_color(0x0, screen_buffers[1]);

    active_screen_buffer = screen_buffers[0];

    MP_STATE_VM(back_fb_data) = mp_obj_new_bytearray_by_ref(SCREEN_BUFFER_SIZE_BYTES, screen_buffers[1]);
    MP_STATE_VM(front_fb_data) = mp_obj_new_bytearray_by_ref(SCREEN_BUFFER_SIZE_BYTES, screen_buffers[0]);

    mp_obj_t framebuf_module = mp_import_name(MP_QSTR_framebuf, mp_const_none, MP_OBJ_NEW_SMALL_INT(0));
    mp_obj_t framebuf_format = mp_load_attr(framebuf_module, MP_QSTR_RGB565);
    mp_obj_t framebuf_constructor = mp_load_attr(framebuf_module, MP_QSTR_FrameBuffer);

    mp_obj_t back_framebuf_params[4] = {
        MP_STATE_VM(back_fb_data),
        mp_obj_new_int(SCREEN_WIDTH),
        mp_obj_new_int(SCREEN_HEIGHT),
        framebuf_format
    };

    mp_obj_t front_framebuf_params[4] = {
        MP_STATE_VM(front_fb_data),
        mp_obj_new_int(SCREEN_WIDTH),
        mp_obj_new_int(SCREEN_HEIGHT),
        framebuf_format
    };

    MP_STATE_VM(back_fb) = mp_call_function_n_kw(framebuf_constructor, 4, 0, back_framebuf_params);
    MP_STATE_VM(front_fb) = mp_call_function_n_kw(framebuf_constructor, 4, 0, front_framebuf_params);
}


void engine_switch_active_screen_buffer(){
    mp_obj_array_t *back_data = MP_STATE_VM(back_fb_data);
    mp_obj_array_t *front_data = MP_STATE_VM(back_fb_data);
    mp_obj_framebuf_t *back = MP_STATE_VM(back_fb);
    mp_obj_framebuf_t *front = MP_STATE_VM(front_fb);

    back_data->items = screen_buffers[active_screen_buffer_index];
    back->buf = screen_buffers[active_screen_buffer_index];

    // Switch which screen buffer should be drawn to
    active_screen_buffer_index = 1 - active_screen_buffer_index;
    active_screen_buffer = screen_buffers[active_screen_buffer_index];

    front_data->items = screen_buffers[active_screen_buffer_index];
    front->buf = screen_buffers[active_screen_buffer_index];
}


void ENGINE_FAST_FUNCTION(engine_display_clear_depth_buffer)(){
    if(depth_buffer != NULL) engine_draw_fill_color(UINT16_MAX, depth_buffer);
}


void engine_display_check_depth_buffer_created(){
    if(depth_buffer == NULL){
        depth_buffer = m_tracked_calloc(1, SCREEN_BUFFER_SIZE_BYTES);
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