#ifndef ENGINE_DISPLAY_DRAW_H
#define ENGINE_DISPLAY_DRAW_H

#include "py/obj.h"
#include <stdint.h>

#include "math/vector3.h"
#include "math/rectangle.h"

// Draw pixel within screen bounds and put into screen buffer
void engine_draw_pixel(uint16_t color, int32_t x, int32_t y);
void engine_draw_pixel_viewport(uint16_t color, int32_t x, int32_t y, int32_t vx, int32_t vy, uint8_t vw, uint8_t vh, int32_t px, int32_t py);

// Draw pixel immediately without checking anything at all
void engine_draw_pixel_unsafe(uint16_t color, int32_t x, int32_t y);

// Draw pixel immediately without checking anything at all
void engine_draw_pixel_unsafe(uint16_t color, int32_t x, int32_t y);

// Fill entire screen buffer with a color
void engine_draw_fill_screen_buffer(uint16_t color, uint16_t *screen_buffer);

// Blits pixels to screen at 'x' and 'y' given 'width' and 'height'
void engine_draw_blit(uint16_t *pixels, int32_t x, int32_t y, uint16_t width, uint16_t height);

// Blits pixels to screen at 'x' and 'y' given 'width' and 'height', after scaling by a factor of (xsc, ysc).
void engine_draw_blit_scale(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc);

// Blits pixels to screen at 'x' and 'y' given 'width' and 'height', after scaling by a factor of (xsc, ysc) and shearing by xsr, then ysr, then xsr2.
void engine_draw_blit_scale_trishear(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc, int32_t xsr, int32_t ysr, int32_t xsr2, int flip);

// Blits pixels to screen at 'x' and 'y' given 'width' and 'height', after scaling by a factor of (xsc, ysc) and rotating by theta in 10-bit bradians.
void engine_draw_blit_scale_rotate(uint16_t *pixels, int32_t x, int32_t y, uint16_t width_log2, uint16_t height, int32_t xsc, int32_t ysc, int16_t theta);

#endif  // ENGINE_DISPLAY_DRAW_H
