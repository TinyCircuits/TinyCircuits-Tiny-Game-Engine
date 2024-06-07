#include "engine_display.h"
#include "engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "debug/debug_print.h"
#include "py/obj.h"


// https://stackoverflow.com/questions/43287103/predefined-macro-to-distinguish-arm-none-eabi-gcc-from-gcc
#ifdef __unix__
    #include "engine_display_driver_unix_sdl.h"
#else
    #include "engine_display_driver_rp2_gc9107.h"
    // #include "engine_display_driver_rp2_st7789.h"
#endif

// Defined in engine_display_common.c
extern uint16_t *active_screen_buffer;


void engine_display_init(){
    ENGINE_PRINTF("EngineDisplay: Setting up...\n");

    engine_init_screen_buffers();

    #ifdef __unix__
        engine_display_sdl_init();
    #else
        engine_display_gc9107_init();
        // engine_display_st7789_init();
    #endif
}


void engine_display_send(){
    // Send the screen buffer to the display
    #ifdef __unix__
        engine_display_sdl_update_screen(active_screen_buffer);
        // engine_display_sdl_update_screen(depth_buffer);
    #else
        engine_display_gc9107_update(active_screen_buffer);
        // engine_display_st7789_update(active_screen_buffer);
    #endif

    engine_switch_active_screen_buffer();

    uint16_t *engine_fill_background = engine_display_get_background();

    // Clear the new active screen buffer
    if(engine_fill_background != NULL){
        engine_draw_fill_buffer(engine_fill_background, active_screen_buffer);
    }else{
        uint16_t engine_fill_color = engine_display_get_color();
        engine_draw_fill_color(engine_fill_color, active_screen_buffer);
    }
}
