#include "engine_display_driver_unix_sdl.h"
#include "engine_display_common.h"
#include "draw/engine_display_draw.h"
#include "debug/debug_print.h"
#include <SDL2/SDL.h>

SDL_Window *window;
SDL_Renderer *window_renderer;      // https://dev.to/noah11012/using-sdl2-2d-accelerated-renderering-1kcb
SDL_Texture* window_frame_buffer;   // https://gamedev.stackexchange.com/questions/157604/how-to-get-access-to-framebuffer-as-a-uint32-t-in-sdl2


void engine_display_sdl_update_screen(uint16_t *screen_buffer_to_render){
    SDL_UpdateTexture(window_frame_buffer , NULL, screen_buffer_to_render, SCREEN_WIDTH*sizeof(uint16_t));
    SDL_RenderClear(window_renderer);
    SDL_RenderCopy(window_renderer, window_frame_buffer, NULL, NULL);
    SDL_RenderPresent(window_renderer);
}


void engine_display_sdl_init(){
    // https://dev.to/noah11012/using-sdl2-opening-a-window-79c
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        ENGINE_ERROR_PRINTF("Failed to initialize the SDL2 library");
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("SDL Video Init Error"));
    }

    window = SDL_CreateWindow("Engine Window",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    
    if(!window){
        ENGINE_ERROR_PRINTF("Failed to create window");
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("SDL Window Init Error"));
    }

    // window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    window_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    window_frame_buffer = SDL_CreateTexture(window_renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_SetWindowSize(window, SCREEN_WIDTH*3, SCREEN_HEIGHT*3);

    engine_display_sdl_update_screen(active_screen_buffer);
}