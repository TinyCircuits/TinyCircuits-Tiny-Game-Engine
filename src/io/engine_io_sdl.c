#include "engine_io_sdl.h"
#include "engine_io_button_codes.h"
#include <SDL2/SDL.h>

static SDL_Event event;

uint16_t sdl_pressed_buttons = 0;

uint16_t engine_io_sdl_pressed_buttons(){
    // Poll queued SDL input events (mouse and keyboard but only keyboard used)
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_KEYDOWN){
            switch(event.key.keysym.sym){
                case SDLK_w:        // DPAD UP
                    sdl_pressed_buttons |= BUTTON_CODE_DPAD_UP;
                break;
                case SDLK_a:        // DPAD LEFT
                    sdl_pressed_buttons |= BUTTON_CODE_DPAD_LEFT;
                break;
                case SDLK_s:        // DPAD DOWN
                    sdl_pressed_buttons |= BUTTON_CODE_DPAD_DOWN;
                break;
                case SDLK_d:        // DPAD RIGHT
                    sdl_pressed_buttons |= BUTTON_CODE_DPAD_RIGHT;
                break;
                case SDLK_PERIOD:   // A
                    sdl_pressed_buttons |= BUTTON_CODE_A;
                break;
                case SDLK_COMMA:    // B
                    sdl_pressed_buttons |= BUTTON_CODE_B;
                break;
                case SDLK_LSHIFT:   // BUMPER LEFT
                    sdl_pressed_buttons |= BUTTON_CODE_BUMPER_LEFT;
                break;
                case SDLK_SPACE:    // BUMPER RIGHT
                    sdl_pressed_buttons |= BUTTON_CODE_BUMPER_RIGHT;
                break;
                case SDLK_RETURN:    // MENU
                    sdl_pressed_buttons |= BUTTON_CODE_MENU;
                break;
            }
        }else if(event.type == SDL_KEYUP){
            // Only get an event when keydown/keyup happens at that time. Can't
            // sample its state all the time like rp3 port to see if currently
            // pressed or not. Need to set bits manually on or off

            switch(event.key.keysym.sym){
                case SDLK_w:        // DPAD UP
                    sdl_pressed_buttons &= ~BUTTON_CODE_DPAD_UP;
                break;
                case SDLK_a:        // DPAD LEFT
                    sdl_pressed_buttons &= ~BUTTON_CODE_DPAD_LEFT;
                break;
                case SDLK_s:        // DPAD DOWN
                    sdl_pressed_buttons &= ~BUTTON_CODE_DPAD_DOWN;
                break;
                case SDLK_d:        // DPAD RIGHT
                    sdl_pressed_buttons &= ~BUTTON_CODE_DPAD_RIGHT;
                break;
                case SDLK_PERIOD:    // A
                    sdl_pressed_buttons &= ~BUTTON_CODE_A;
                break;
                case SDLK_COMMA:   // B
                    sdl_pressed_buttons &= ~BUTTON_CODE_B;
                break;
                case SDLK_LSHIFT:   // BUMPER LEFT
                    sdl_pressed_buttons &= ~BUTTON_CODE_BUMPER_LEFT;
                break;
                case SDLK_SPACE:    // BUMPER RIGHT
                    sdl_pressed_buttons &= ~BUTTON_CODE_BUMPER_RIGHT;
                break;
                case SDLK_RETURN:    // MENU
                    sdl_pressed_buttons &= ~BUTTON_CODE_MENU;
                break;
            }
        }
    }
    return sdl_pressed_buttons;
}
