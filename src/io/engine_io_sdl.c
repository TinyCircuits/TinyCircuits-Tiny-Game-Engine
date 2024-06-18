#include "engine_io_sdl.h"
#include "engine_io_common.h"
#include <SDL2/SDL.h>

static SDL_Event event;

void engine_io_sdl_update_pressed_mask(){
    // Poll queued SDL input events (mouse and keyboard but only keyboard used)
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_KEYDOWN){
            switch(event.key.keysym.sym){
                case SDLK_w:        // DPAD UP
                    pressed_buttons |= BUTTON_DPAD_UP;
                break;
                case SDLK_a:        // DPAD LEFT
                    pressed_buttons |= BUTTON_DPAD_LEFT;
                break;
                case SDLK_s:        // DPAD DOWN
                    pressed_buttons |= BUTTON_DPAD_DOWN;
                break;
                case SDLK_d:        // DPAD RIGHT
                    pressed_buttons |= BUTTON_DPAD_RIGHT;
                break;
                case SDLK_PERIOD:   // A
                    pressed_buttons |= BUTTON_A;
                break;
                case SDLK_COMMA:    // B
                    pressed_buttons |= BUTTON_B;
                break;
                case SDLK_LSHIFT:   // BUMPER LEFT
                    pressed_buttons |= BUTTON_BUMPER_LEFT;
                break;
                case SDLK_SPACE:    // BUMPER RIGHT
                    pressed_buttons |= BUTTON_BUMPER_RIGHT;
                break;
                case SDLK_RETURN:    // MENU
                    pressed_buttons |= BUTTON_MENU;
                break;
            }
        }else if(event.type == SDL_KEYUP){
            // Only get an event when keydown/keyup happens at that time. Can't
            // sample its state all the time like rp3 port to see if currently
            // pressed or not. Need to set bits manually on or off

            switch(event.key.keysym.sym){
                case SDLK_w:        // DPAD UP
                    pressed_buttons &= ~BUTTON_DPAD_UP;
                break;
                case SDLK_a:        // DPAD LEFT
                    pressed_buttons &= ~BUTTON_DPAD_LEFT;
                break;
                case SDLK_s:        // DPAD DOWN
                    pressed_buttons &= ~BUTTON_DPAD_DOWN;
                break;
                case SDLK_d:        // DPAD RIGHT
                    pressed_buttons &= ~BUTTON_DPAD_RIGHT;
                break;
                case SDLK_PERIOD:    // A
                    pressed_buttons &= ~BUTTON_A;
                break;
                case SDLK_COMMA:   // B
                    pressed_buttons &= ~BUTTON_B;
                break;
                case SDLK_LSHIFT:   // BUMPER LEFT
                    pressed_buttons &= ~BUTTON_BUMPER_LEFT;
                break;
                case SDLK_SPACE:    // BUMPER RIGHT
                    pressed_buttons &= ~BUTTON_BUMPER_RIGHT;
                break;
                case SDLK_RETURN:    // MENU
                    pressed_buttons &= ~BUTTON_MENU;
                break;
            }
        }
    }
}
