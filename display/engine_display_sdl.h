#ifndef ENGINE_DISPLAY_SDL_H
#define ENGINE_DISPLAY_SDL_H

#include <stdint.h>

void engine_display_sdl_init();
void sdl_update_screen(uint16_t *screen_buffer_to_render);


#endif  // ENGINE_DISPLAY_H