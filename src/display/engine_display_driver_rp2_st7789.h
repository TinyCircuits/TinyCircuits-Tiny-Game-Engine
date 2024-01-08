#ifndef ENGINE_DISPLAY_DRIVER_RP2_ST7789_H
#define ENGINE_DISPLAY_DRIVER_RP2_ST7789_H

#include <stdint.h>

// https://github.com/ArmDeveloperEcosystem/st7789-library-for-pico/blob/main/src/st7789.c
// https://github.com/ArmDeveloperEcosystem/st7789-library-for-pico/blob/main/src/include/pico/st7789.h
// https://github.com/raspberrypi/pico-examples/blob/master/spi/spi_dma/spi_dma.c
void engine_display_st7789_init();
void engine_display_st7789_update(uint16_t *screen_buffer_to_render);

#endif  // ENGINE_DISPLAY_DRIVER_RP2_ST7789_H
