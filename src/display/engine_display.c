#include "engine_display.h"
#include "engine_display_common.h"
#include "draw/engine_display_draw.h"


// https://stackoverflow.com/questions/43287103/predefined-macro-to-distinguish-arm-none-eabi-gcc-from-gcc
#ifdef __unix__
    #include "engine_display_driver_unix_sdl.h"
#else
    #include "engine_display_driver_rp2_gc9107.h"
    // #include "engine_display_driver_rp2_st7789.h"
#endif


void engine_display_init(){
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
    #else
        engine_display_gc9107_update(engine_get_active_screen_buffer());
        // engine_display_st7789_update(engine_get_active_screen_buffer());
    #endif

    engine_switch_active_screen_buffer();

    // Clear the new active screen buffer
    engine_draw_fill(engine_fill_color, engine_get_active_screen_buffer());
}
