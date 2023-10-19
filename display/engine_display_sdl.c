#include "engine_display_sdl.h"
#include "utility/debug_print.h"
#include <SDL2/SDL.h>

SDL_Window *window;
SDL_Surface *window_surface;
SDL_Rect window_rect;


void engine_display_sdl_init(){
    // https://dev.to/noah11012/using-sdl2-opening-a-window-79c
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        ENGINE_ERROR_PRINTF("Failed to initialize the SDL2 library");
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("SDL Video Init Error"));
    }

    window = SDL_CreateWindow("Engine Window",
                                          600,
                                          400,
                                          128, 128,
                                          SDL_WINDOW_ALWAYS_ON_TOP);
    
    if(!window){
        ENGINE_ERROR_PRINTF("Failed to create window");
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("SDL Window Init Error"));
    }

    window_surface = SDL_GetWindowSurface(window);

    if(!window_surface){
        ENGINE_ERROR_PRINTF("Failed to get the surface from the window");
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("SDL Window Surface Init Error"));
    }

    window_rect.x = 0;
    window_rect.y = 0;
    window_rect.w = 128;
    window_rect.h = 128;

    SDL_FillRect(window_surface, &window_rect, 0x00000000);

    SDL_UpdateWindowSurface(window);
}