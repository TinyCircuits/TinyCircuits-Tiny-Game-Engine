#ifndef ENGINE_DISPLAY_COMMON
#define ENGINE_DISPLAY_COMMON

#include "py/obj.h"
#include <stdint.h>
#include <stdbool.h>

MP_REGISTER_ROOT_POINTER(mp_obj_t back_fb_data);
MP_REGISTER_ROOT_POINTER(mp_obj_t back_fb);

MP_REGISTER_ROOT_POINTER(mp_obj_t front_fb_data);
MP_REGISTER_ROOT_POINTER(mp_obj_t front_fb);


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define SCREEN_WIDTH_HALF SCREEN_WIDTH * 0.5f
#define SCREEN_HEIGHT_HALF SCREEN_HEIGHT * 0.5f

#define SCREEN_WIDTH_MINUS_1 SCREEN_WIDTH-1
#define SCREEN_HEIGHT_MINUS_1 SCREEN_HEIGHT-1           

#define SCREEN_WIDTH_INVERSE 1.0f / SCREEN_WIDTH
#define SCREEN_HEIGHT_INVERSE 1.0f / SCREEN_HEIGHT

#define SCREEN_BUFFER_SIZE_PIXELS SCREEN_WIDTH*SCREEN_HEIGHT
#define SCREEN_BUFFER_SIZE_BYTES SCREEN_BUFFER_SIZE_PIXELS*2 // Number of pixels times 2 (16-bit pixels) is the number of bytes in a screen buffer


void engine_display_set_fill_color(uint16_t color);
void engine_display_set_fill_background(uint16_t *data);
void engine_display_reset_fills();

uint16_t *engine_display_get_background();
uint16_t engine_display_get_color();

void engine_display_init_framebuffers();

void engine_init_screen_buffers();

// Switches active screen buffer
void engine_switch_active_screen_buffer();

// Resets all elements to 0x0000
void engine_display_clear_depth_buffer();

// Checks that the depth buffer has been created, if not, creates it
void engine_display_check_depth_buffer_created();

// Frees the depth buffer (should be used on engine reset)
void engine_display_free_depth_buffer();

uint16_t *engine_display_get_depth_buffer();

// Returns true if the passed depth is lower/closer
// than the depth stored there before, also stores it if true.
// Returns false if did not store it because it was lower
bool engine_display_store_check_depth_index(uint16_t index, uint16_t depth);
bool engine_display_store_check_depth(uint8_t sx, uint8_t sy, uint16_t depth);

#endif  // ENGINE_DISPLAY_COMMON