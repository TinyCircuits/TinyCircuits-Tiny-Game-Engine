#ifndef ENGINE_DISPLAY_DRAW_H
#define ENGINE_DISPLAY_DRAW_H

#include "py/obj.h"
#include <stdint.h>
#include "engine_color.h"
#include "resources/engine_font_resource.h"
#include "utility/engine_defines.h"

// Color that is passed to draw function indicating that no transparency is needed
#define ENGINE_NO_TRANSPARENCY_COLOR 0b0000100000100001


// Fills entire screen buffer with 'color'
void ENGINE_FAST_FUNCTION(engine_draw_fill_color)(uint16_t color, uint16_t *screen_buffer);

// Fills entire screen buffer with 'src_buffer'
void ENGINE_FAST_FUNCTION(engine_draw_fill_buffer)(uint16_t* src_buffer, uint16_t *screen_buffer);

// Sets a single pixel in the screen buffer to 'color'
void ENGINE_FAST_FUNCTION(engine_draw_pixel)(uint16_t color, int32_t x, int32_t y, float alpha, uint8_t *shader, uint8_t shader_len);

void engine_draw_line(uint16_t color, float x_start, float y_start, float x_end, float y_end, mp_obj_t camera_node_base, float alpha);

void engine_draw_blit(uint16_t *pixels, float center_x, float center_y, uint32_t window_width, uint32_t window_height, uint32_t pixels_stride, float x_scale, float y_scale, float rotation_radians, uint16_t transparent_color, float alpha);

void engine_draw_rect(uint16_t color, float center_x, float center_y, uint32_t width, uint32_t height, float x_scale, float y_scale, float rotation_radians, float alpha);

void engine_draw_outline_circle(uint16_t color, float center_x, float center_y, float radius, float alpha);

void engine_draw_filled_circle(uint16_t color, float center_x, float center_y, float radius, float alpha);

void engine_draw_text(font_resource_class_obj_t *font, mp_obj_t text, float center_x, float center_y, float text_box_width, float text_box_height, float x_scale, float y_scale, float rotation_radians, float alpha);

#endif  // ENGINE_DISPLAY_DRAW_H