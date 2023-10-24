#ifndef ENGINE_DISPLAY_DRAW_H
#define ENGINE_DISPLAY_DRAW_H

#include <stdint.h>

// void engine_draw_pixel(uint16_t color, );

// Fill entire screen buffer with a color
void engine_draw_fill(uint16_t color);

// Blits pixels to screen at 'x' and 'y' given 'width' and 'height'
void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height);

#endif  // ENGINE_DISPLAY_DRAW_H