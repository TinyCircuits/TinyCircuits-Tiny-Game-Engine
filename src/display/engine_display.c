#include "engine_display.h"
#include "engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "debug/debug_print.h"
#include "py/obj.h"
#include "math/engine_math.h"


// https://stackoverflow.com/questions/43287103/predefined-macro-to-distinguish-arm-none-eabi-gcc-from-gcc
#if defined(__EMSCRIPTEN__)
    #include <emscripten.h>

    EM_JS(void, engine_display_web_update_screen, (uint16_t *screen_buffer_to_render), {
        self.update_display(screen_buffer_to_render); // Call Javascript function that updates canvas
    });
#elif defined(__unix__)
    #include "engine_display_driver_unix_sdl.h"
#elif defined(__arm__)
    #include "engine_display_driver_rp2_gc9107.h"
#else
    #warning "PLATFORM COMPILE-TIME ERROR, EngineDisplay: Unsupported platform"
#endif


// Defined in engine_display_common.c
extern uint16_t *active_screen_buffer;
float screen_brightness = 1.0f;

void engine_display_init(){
    ENGINE_PRINTF("EngineDisplay: Setting up...\n");

    engine_init_screen_buffers();

    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        engine_display_sdl_init();
    #elif defined(__arm__)
        engine_display_gc9107_init();
    #endif
}


void engine_display_apply_brightness(float brightness){
    screen_brightness = brightness;

    // Make sure users can't turn their screen completely off
    brightness = engine_math_clamp(brightness, 0.05f, 1.0f);

    #if defined(__EMSCRIPTEN__)

    #elif defined(__unix__)
        engine_display_sdl_apply_brightness(brightness);
    #elif defined(__arm__)
        engine_display_gc9107_apply_brightness(brightness);
    #endif
}


float engine_display_get_brightness(){
    return screen_brightness;
}


void engine_display_clear(){
    uint16_t *engine_fill_background = engine_display_get_background();

    // Clear the new active screen buffer
    if(engine_fill_background != NULL){
        engine_draw_fill_buffer(engine_fill_background, active_screen_buffer);
    }else{
        uint16_t engine_fill_color = engine_display_get_color();
        engine_draw_fill_color(engine_fill_color, active_screen_buffer);
    }
}


void engine_display_send(){
    // Send the screen buffer to the display
    // Send the screen buffer to the display
    #if defined(__EMSCRIPTEN__)
        engine_display_web_update_screen(active_screen_buffer);
    #elif defined(__unix__)
        engine_display_sdl_update_screen(active_screen_buffer);
    #elif defined(__arm__)
        engine_display_gc9107_update(active_screen_buffer);
    #endif

    engine_switch_active_screen_buffer();
}
