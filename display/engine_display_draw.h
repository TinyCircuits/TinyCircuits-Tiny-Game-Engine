#ifndef ENGINE_DISPLAY_DRAW_H
#define ENGINE_DISPLAY_DRAW_H

#include <stdint.h>

// Draw pixel within screen bounds and put into screen buffer
void engine_draw_pixel(uint16_t color, int32_t x, int32_t y);

// Fill entire screen buffer with a color
void engine_draw_fill_screen_buffer(uint16_t color, uint16_t *screen_buffer);

// Blits pixels to screen at 'x' and 'y' given 'width' and 'height'
void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height);

#endif  // ENGINE_DISPLAY_DRAW_H