#ifndef ENGINE_DISPLAY_DRIVER_RP2_GC9107_H
#define ENGINE_DISPLAY_DRIVER_RP2_GC9107_H

#include <stdint.h>

// Driver reference implementation: https://www.buydisplay.com/0-85-inch-128x128-ips-tft-lcd-display-4-wire-spi-gc9107-controller (8051 Interfacing Demo Code)
//                                  https://www.buydisplay.com/8051/ER-TFT0.85-1_8051_Tutorial.zip
void engine_display_gc9107_init();
void engine_display_gc9107_update(uint16_t *screen_buffer_to_render);

#endif  // ENGINE_DISPLAY_DRIVER_RP2_GC9107_H
