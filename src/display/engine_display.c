#include "engine_display.h"
#include "engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "debug/debug_print.h"


// https://stackoverflow.com/questions/43287103/predefined-macro-to-distinguish-arm-none-eabi-gcc-from-gcc
#ifdef __unix__
    #include "engine_display_driver_unix_sdl.h"
#else
    #include "engine_display_driver_rp2_gc9107.h"
    // #include "engine_display_driver_rp2_st7789.h"
#endif


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
        engine_display_sdl_update_screen(engine_get_active_screen_buffer());
        // engine_display_sdl_update_screen(depth_buffer);
    #else
        engine_display_gc9107_update(engine_get_active_screen_buffer());
        // engine_display_st7789_update(engine_get_active_screen_buffer());
    #endif

    engine_switch_active_screen_buffer();

    // Clear the new active screen buffer
    if(engine_fill_background != NULL){
        engine_draw_fill_buffer(engine_fill_background, engine_get_active_screen_buffer());
    }else{
        engine_draw_fill_color(engine_fill_color, engine_get_active_screen_buffer());
    }
}
