#ifndef ENGINE_DISPLAY_COMMON
#define ENGINE_DISPLAY_COMMON

#include <stdint.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SCREEN_BUFFER_SIZE SCREEN_WIDTH*SCREEN_HEIGHT*2 // Number of pixels times 2 (16-bit pixels) is the number of bytes in a screen buffer

// Returns the active screen buffer
uint16_t *engine_get_active_screen_buffer();

// Switches active screen buffer
void engine_switch_active_screen_buffer();

#endif  // ENGINE_DISPLAY_COMMON