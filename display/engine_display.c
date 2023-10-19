#include "engine_display.h"
#include "engine_display_common.h"


// https://stackoverflow.com/questions/43287103/predefined-macro-to-distinguish-arm-none-eabi-gcc-from-gcc
#ifdef __unix__
    #include "engine_display_sdl.h"
#else
    #include "utility/debug_print.h"
#endif


void engine_display_init(){
    #ifdef __unix__
        engine_display_sdl_init();
    #else
        ENGINE_WARNING_PRINTF("Display init not implemented for this platform");
    #endif
}


void engine_display_send(){
    // Send the screen buffer to the display
    // TODO

    // Switch which screen buffer should be drawn to
    if(active_screen_buffer_index == 0){
        active_screen_buffer_index = 1;
    }else{
        active_screen_buffer_index = 0;
    }

    active_screen_buffer = dual_screen_buffers[active_screen_buffer_index];
}