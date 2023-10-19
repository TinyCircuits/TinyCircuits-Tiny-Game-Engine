#ifndef ENGINE_DISPLAY_COMMON
#define ENGINE_DISPLAY_COMMON

#include <stdint.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

// Two screen buffers are allocated so that one can be
// getting sent to the screen using the independent DMA
// hardware while the other is being used by the CPU
// to draw the next frame
static uint16_t dual_screen_buffers[2][SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(uint16_t)];

// The current screen buffer that should be getting drawn to (the other
// one is likely being sent to the screen while this is active)
static uint16_t *active_screen_buffer;

// The current index of the 'active_screen_buffer' in 'dual_screen_buffers'
// (gets switched when the screen buffer is sent out over DMA)
static uint8_t active_screen_buffer_index = 0;

#endif  // ENGINE_DISPLAY_COMMON