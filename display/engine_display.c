#include "engine_display.h"
#include "engine_display_common.h"
#include "engine_display_draw.h"


// https://stackoverflow.com/questions/43287103/predefined-macro-to-distinguish-arm-none-eabi-gcc-from-gcc
#ifdef __unix__
    #include "engine_display_sdl.h"
#else
    #include "utility/debug_print.h"
#endif


void engine_display_init(){
    engine_init_screen_buffers();

    #ifdef __unix__
        engine_display_sdl_init();
    #else
        ENGINE_WARNING_PRINTF("Display init not implemented for this platform");
    #endif
}


void engine_display_send(){
    // Send the screen buffer to the display
    #ifdef __unix__
        sdl_update_screen(engine_get_active_screen_buffer());
    #else

    #endif

    engine_switch_active_screen_buffer();

    // Clear the new active screen buffer
    engine_draw_fill_screen_buffer(0x0, engine_get_active_screen_buffer());
}