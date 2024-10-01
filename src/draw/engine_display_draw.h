#ifndef ENGINE_DISPLAY_DRAW_H
#define ENGINE_DISPLAY_DRAW_H

#include "py/obj.h"
#include <stdint.h>
#include "engine_color.h"
#include "resources/engine_font_resource.h"
#include "utility/engine_defines.h"
#include "draw/engine_shader.h"

// Color that is passed to draw function indicating that no transparency is needed
#define ENGINE_NO_TRANSPARENCY_COLOR 0b0000100000100001


// Fills entire screen buffer with 'color'
void ENGINE_FAST_FUNCTION(engine_draw_fill_color)(uint16_t color, uint16_t *screen_buffer);

// Fills entire screen buffer with 'src_buffer'
void ENGINE_FAST_FUNCTION(engine_draw_fill_buffer)(uint16_t* src_buffer, uint16_t *screen_buffer);

// Sets a single pixel in the screen buffer to 'color'
void ENGINE_FAST_FUNCTION(engine_draw_pixel)(uint16_t color, int32_t x, int32_t y, float alpha, engine_shader_t *shader);

void ENGINE_FAST_FUNCTION(engine_draw_pixel_no_check)(uint16_t color, int32_t x, int32_t y, float alpha, engine_shader_t *shader);

void engine_draw_line(uint16_t color, float x_start, float y_start, float x_end, float y_end, mp_obj_t camera_node_base, float alpha, engine_shader_t *shader);

void engine_draw_blit(texture_resource_class_obj_t *texture, uint32_t offset, float center_x, float center_y, int32_t window_width, int32_t window_height, uint32_t pixels_stride, float x_scale, float y_scale, float rotation_radians, uint16_t transparent_color, float alpha, engine_shader_t *shader);

void engine_draw_blit_depth(texture_resource_class_obj_t *texture, uint32_t offset, float center_x, float center_y, int32_t window_width, int32_t window_height, uint32_t pixels_stride, float x_scale, float y_scale, float rotation_radians, uint16_t transparent_color, float alpha, uint16_t depth, engine_shader_t *shader);

void engine_draw_rect(uint16_t color, float center_x, float center_y, int32_t width, int32_t height, float x_scale, float y_scale, float rotation_radians, float alpha, engine_shader_t *shader);

void engine_draw_outline_circle(uint16_t color, float center_x, float center_y, float radius, float alpha, engine_shader_t *shader);

void engine_draw_filled_circle(uint16_t color, float center_x, float center_y, float radius, float alpha, engine_shader_t *shader);

void engine_draw_text(font_resource_class_obj_t *font, mp_obj_t text, float center_x, float center_y, float text_box_width, float text_box_height, float letter_spacing, float line_spacing, float x_scale, float y_scale, float rotation_radians, float alpha, engine_shader_t *shader);

void engine_draw_filled_triangle(uint16_t color, float x0, float y0, float x1, float y1, float x2, float y2, float alpha, engine_shader_t *shader);

void engine_draw_filled_triangle_depth(uint16_t color, float ax, float ay, uint16_t depth_az, float bx, float by, uint16_t depth_bz, float cx, float cy, uint16_t depth_cz, float alpha, engine_shader_t *shader);

#endif  // ENGINE_DISPLAY_DRAW_H