#ifndef ENGINE_DISPLAY_COMMON
#define ENGINE_DISPLAY_COMMON

#include <stdint.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH_MINUS_1 127
#define SCREEN_HEIGHT_MINUS_1 127
#define SCREEN_BUFFER_SIZE_PIXELS SCREEN_WIDTH*SCREEN_HEIGHT
#define SCREEN_BUFFER_SIZE_BYTES SCREEN_BUFFER_SIZE_PIXELS*2 // Number of pixels times 2 (16-bit pixels) is the number of bytes in a screen buffer

// The fill color that is used to clear the screen
uint16_t engine_fill_color;
uint16_t *engine_fill_background;

void engine_init_screen_buffers();

// Returns the active screen buffer
uint16_t *engine_get_active_screen_buffer();

// Switches active screen buffer
void engine_switch_active_screen_buffer();

#endif  // ENGINE_DISPLAY_COMMON