#include "engine_input_sdl.h"
#include "engine_input_common.h"
#include <SDL2/SDL.h>

static SDL_Event event;

void engine_input_sdl_update_pressed_mask(){
    // Poll queued SDL input events (mouse and keyboard but only keyboard used)
    while(SDL_PollEvent(&event)){
        if(event.type == SDL_KEYDOWN){
            switch(event.key.keysym.sym){
                case SDLK_w:        // DPAD UP
                    engine_input_pressed_buttons |= BUTTON_DPAD_UP;
                break;
                case SDLK_a:        // DPAD LEFT
                    engine_input_pressed_buttons |= BUTTON_DPAD_LEFT;
                break;
                case SDLK_s:        // DPAD DOWN
                    engine_input_pressed_buttons |= BUTTON_DPAD_DOWN;
                break;
                case SDLK_d:        // DPAD RIGHT
                    engine_input_pressed_buttons |= BUTTON_DPAD_RIGHT;
                break;
                case SDLK_COMMA:    // A
                    engine_input_pressed_buttons |= BUTTON_A;
                break;
                case SDLK_PERIOD:   // B
                    engine_input_pressed_buttons |= BUTTON_B;
                break;
                case SDLK_LSHIFT:   // BUMPER LEFT
                    engine_input_pressed_buttons |= BUTTON_BUMPER_LEFT;
                break;
                case SDLK_SPACE:    // BUMPER RIGHT
                    engine_input_pressed_buttons |= BUTTON_BUMPER_RIGHT;
                break;
                case SDLK_RETURN:    // MENU
                    engine_input_pressed_buttons |= BUTTON_MENU;
                break;
            }
        }else if(event.type == SDL_KEYUP){
            switch(event.key.keysym.sym){
                case SDLK_w:        // DPAD UP
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_DPAD_UP);
                break;
                case SDLK_a:        // DPAD LEFT
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_DPAD_LEFT);
                break;
                case SDLK_s:        // DPAD DOWN
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_DPAD_DOWN);
                break;
                case SDLK_d:        // DPAD RIGHT
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_DPAD_RIGHT);
                break;
                case SDLK_COMMA:    // A
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_A);
                break;
                case SDLK_PERIOD:   // B
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_B);
                break;
                case SDLK_LSHIFT:   // BUMPER LEFT
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_BUMPER_LEFT);
                break;
                case SDLK_SPACE:    // BUMPER RIGHT
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_BUMPER_RIGHT);
                break;
                case SDLK_RETURN:    // MENU
                    engine_input_pressed_buttons = engine_input_pressed_buttons & ~(BUTTON_MENU);
                break;
            }
        }
    }
}
