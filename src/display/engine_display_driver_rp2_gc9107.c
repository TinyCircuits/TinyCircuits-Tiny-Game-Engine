#include "engine_display_driver_rp2_gc9107.h"
#include "engine_display_common.h"
#include "debug/debug_print.h"
#include "engine_display.h"

#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pwm.pio.h"
#include "io/engine_io_rp3.h"


// Based on max from Bodmer: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/User_Setup.h#L366
#define GC9107_SPI_MHZ 80 * 1000 * 1000


// Who add these numbers? Not sure, if you don't then the window address is wrong
// const uint16_t WINDOW_ADDR_X1 = 0 + 2;
// const uint16_t WINDOW_ADDR_X2 = SCREEN_WIDTH + 1;
// const uint16_t WINDOW_ADDR_Y1 = 0 + 1;
// const uint16_t WINDOW_ADDR_Y2 = SCREEN_HEIGHT;
const uint16_t WINDOW_ADDR_X1 = 0;
const uint16_t WINDOW_ADDR_X2 = SCREEN_WIDTH_MINUS_1;
const uint16_t WINDOW_ADDR_Y1 = 0;
const uint16_t WINDOW_ADDR_Y2 = SCREEN_HEIGHT_MINUS_1;

int dma_tx;
dma_channel_config dma_config;
uint16_t *txbuf = NULL;


PIO pio = pio0;
int sm = 0;


// Write `period` to the input shift register
void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

// Write `level` to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
    pio_sm_put_blocking(pio, sm, level);
}


static void gc9107_write_cmd(uint8_t cmd, const uint8_t* data, size_t length){
    // Interesting note on performance: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/TFT_eSPI.cpp#L3416-L3417
    // Set to 8-bit SPI during cmd sending
    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // Select screen chip and put into cmd mode (see pin connection reference)
    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 0);
    gpio_put(PIN_GP16__TO__DC, 0);

    // Write the command to the driver through SPI
    spi_write_blocking(spi0, &cmd, sizeof(cmd));

    // If there's also data, write that but put chip in data mode (see pin connection reference)
    gpio_put(PIN_GP16__TO__DC, 1);
    if(length > 0){
        spi_write_blocking(spi0, data, length);
    }

    // Now that everything is sent, make sure we're in data mode (and that the screen is not selected? idk)
    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 1);

    // Interesting note on performance: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/TFT_eSPI.cpp#L3416-L3417
    // Set back to 16-bit SPI
    spi_set_format(spi0, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}


void gc9107_reset_window(){
    gc9107_write_cmd(0x36, (uint8_t[]){ 0x00 }, 1);
    gc9107_write_cmd(0x2a, (uint8_t[]){ WINDOW_ADDR_X1>>8, WINDOW_ADDR_X1, WINDOW_ADDR_X2>>8, WINDOW_ADDR_X2 }, 4);
    gc9107_write_cmd(0x2b, (uint8_t[]){ WINDOW_ADDR_Y1>>8, WINDOW_ADDR_Y1, WINDOW_ADDR_Y2>>8, WINDOW_ADDR_Y2 }, 4);
    gc9107_write_cmd(0x2c, NULL, 0);
}


void engine_display_gc9107_apply_brightness(float brightness){
    pio_pwm_set_level(pio, sm, (uint32_t)(255.0f*brightness));
}


void engine_display_gc9107_init(){
    ENGINE_INFO_PRINTF("Setting up GC9107 screen");

    // Init SPI
    ENGINE_INFO_PRINTF("Enabling SPI");
    spi_init(spi0, GC9107_SPI_MHZ);

    // Init pins (some are controlled through SPI peripheral
    // and some are controlled through code using GPIO)
    ENGINE_INFO_PRINTF("Enabling pins");
    gpio_set_function(PIN_GP19_SPI0_TX__TO__SDA, GPIO_FUNC_SPI);
    gpio_set_function(PIN_GP18_SPI0_SCK__TO__CLK, GPIO_FUNC_SPI);

    // Setup GPIO
    gpio_init(PIN_GP17_SPI0_CSn__TO__CS);               // Although part of the SPI port, we'll control chip select manually
    gpio_init(PIN_GP16__TO__DC);
    gpio_init(PIN_GP4__TO__RST);
    gpio_set_dir(PIN_GP17_SPI0_CSn__TO__CS, GPIO_OUT);
    gpio_set_dir(PIN_GP16__TO__DC, GPIO_OUT);
    gpio_set_dir(PIN_GP4__TO__RST, GPIO_OUT);

    // https://github.com/raspberrypi/pico-examples/blob/master/pio/pwm/pwm.c
    // GPIO 7 and 23 share the same slice, use PIO for BL PWM
    uint offset = pio_add_program(pio, &pwm_program);
    pwm_program_init(pio, sm, offset, PIN_GP7_PIO_PWM__TO__BL);
    pio_pwm_set_period(pio, sm, 255);

    // Do the init sequence
    engine_display_gc9107_apply_brightness(0.0f);   // Backlight off before all init
    
    // New demo code from less wide connector screen version
    // https://www.buydisplay.com/square-0-85-inch-128x128-ips-tft-lcd-display-4-wire-spi-gc9107
    // https://www.buydisplay.com/8051/ER-TFT0.85-2_8051_Tutorial.zip
    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 1);
    sleep_ms(5);
    gpio_put(PIN_GP4__TO__RST, 0);
    sleep_ms(50);
    gpio_put(PIN_GP4__TO__RST, 1);
    sleep_ms(120);

    gc9107_write_cmd(0xFE, NULL, 0);                    // Inter register enable 1 
    gc9107_write_cmd(0xEF, NULL, 0);                    // Inter register enable 2

    gc9107_write_cmd(0xB0, (uint8_t[]){ 0xC0 }, 1);     // ?
    gc9107_write_cmd(0xB1, (uint8_t[]){ 0x80 }, 1);     // ?
    gc9107_write_cmd(0xB2, (uint8_t[]){ 0x2F }, 1);     // ?
    gc9107_write_cmd(0xB3, (uint8_t[]){ 0x03 }, 1);     // ?
    gc9107_write_cmd(0xB7, (uint8_t[]){ 0x01 }, 1);     // ?
    gc9107_write_cmd(0xB6, (uint8_t[]){ 0x19 }, 1);     // ?

    gc9107_write_cmd(0xAC, (uint8_t[]){ 0xC8 }, 1);     // Complement Principle of RGB 5, 6, 5
    gc9107_write_cmd(0xAB, (uint8_t[]){ 0x0f }, 1);     // ?

    gc9107_write_cmd(0x3A, (uint8_t[]){ 0x05 }, 1);     // COLMOD: Pixel Format Set

    gc9107_write_cmd(0xB4, (uint8_t[]){ 0x04 }, 1);     // ?

    gc9107_write_cmd(0xA8, (uint8_t[]){ 0x07 }, 1);     // Frame Rate Set

    gc9107_write_cmd(0xB8, (uint8_t[]){ 0x08 }, 1);     // ?

    gc9107_write_cmd(0xE7, (uint8_t[]){ 0x5A }, 1);     // VREG_CTL
    gc9107_write_cmd(0xE8, (uint8_t[]){ 0x23 }, 1);     // VGH_SET
    gc9107_write_cmd(0xE9, (uint8_t[]){ 0x47 }, 1);     // VGL_SET
    gc9107_write_cmd(0xEA, (uint8_t[]){ 0x99 }, 1);     // VGH_VGL_CLK

    gc9107_write_cmd(0xC6, (uint8_t[]){ 0x30 }, 1);     // ?
    gc9107_write_cmd(0xC7, (uint8_t[]){ 0x1F }, 1);     // ?

    gc9107_write_cmd(0xF0, (uint8_t[]){ 0x05, 0x1D, 0x51, 0x2F, 0x85, 0x2A, 0x11, 0x62, 0x00, 0x07, 0x07, 0x0F, 0x08, 0x1F }, 14);  // SET_GAMMA1
    gc9107_write_cmd(0xF1, (uint8_t[]){ 0x2E, 0x41, 0x62, 0x56, 0xA5, 0x3A, 0x3f, 0x60, 0x0F, 0x07, 0x0A, 0x18, 0x18, 0x1D }, 14);  // SET_GAMMA2

    gc9107_write_cmd(0x11, NULL, 0);
    sleep_ms(120);
    gc9107_write_cmd(0x29, NULL, 0);
    sleep_ms(10);

    // // OLD demo code from wider connector screen version
    // https://www.buydisplay.com/0-85-inch-128x128-ips-tft-lcd-display-4-wire-spi-gc9107-controller
    // https://www.buydisplay.com/8051/ER-TFT0.85-1_8051_Tutorial.zip
    // gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 1);
    // sleep_ms(5);
    // gpio_put(PIN_GP4__TO__RST, 0);
    // sleep_ms(50);
    // gpio_put(PIN_GP4__TO__RST, 1);
    // sleep_ms(120);

    // gc9107_write_cmd(0xB0, (uint8_t[]){ 0xC0 }, 1);
    // gc9107_write_cmd(0xB2, (uint8_t[]){ 0x2F }, 1);
    // gc9107_write_cmd(0xB3, (uint8_t[]){ 0x03 }, 1);
    // gc9107_write_cmd(0xB6, (uint8_t[]){ 0x19 }, 1);
    // gc9107_write_cmd(0xB7, (uint8_t[]){ 0x01 }, 1);

    // gc9107_write_cmd(0xAC, (uint8_t[]){ 0xCB }, 1);
    // gc9107_write_cmd(0xAB, (uint8_t[]){ 0x0e }, 1);

    // gc9107_write_cmd(0xB4, (uint8_t[]){ 0x04 }, 1);

    // gc9107_write_cmd(0xA8, (uint8_t[]){ 0x19 }, 1);

    // gc9107_write_cmd(0x3A, (uint8_t[]){ 0x05 }, 1);

    // gc9107_write_cmd(0xb8, (uint8_t[]){ 0x08 }, 1);

    // gc9107_write_cmd(0xE8, (uint8_t[]){ 0x24 }, 1);

    // gc9107_write_cmd(0xE9, (uint8_t[]){ 0x48 }, 1);

    // gc9107_write_cmd(0xea, (uint8_t[]){ 0x22 }, 1);

    // gc9107_write_cmd(0xC6, (uint8_t[]){ 0x30 }, 1);
    // gc9107_write_cmd(0xC7, (uint8_t[]){ 0x18 }, 1);

    // // Gamma, leave not set, use defaults
    // // gc9107_write_cmd(0xF0, (uint8_t[]){ 0x1F, 0x28, 0x04, 0x3E, 0x2A, 0x2E, 0x20, 0x00, 0x0C, 0x06, 0x00, 0x1C, 0x1F, 0x0f }, 14);

    // // gc9107_write_cmd(0xF1, (uint8_t[]){ 0x00, 0x2D, 0x2F, 0x3C, 0x6F, 0x1C, 0x0B, 0x00, 0x00, 0x00, 0x07, 0x0D, 0x11, 0x0f }, 14);

    // gc9107_write_cmd(0x21, NULL, 0);


    // gc9107_write_cmd(0x11, NULL, 0);
    // sleep_ms(120);
    // gc9107_write_cmd(0x29, NULL, 0);
    // sleep_ms(10);
    // // OLD demo code from wider connector screen version

    // Grab unused dma channel for SPI TX
    ENGINE_INFO_PRINTF("Enabling DMA for 16-bit transfers");
    dma_tx = dma_claim_unused_channel(true);

    // Configure the DMA channel (for SPI TX)
    dma_config = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_dreq(&dma_config, DREQ_SPI0_TX);

    engine_display_gc9107_apply_brightness(engine_display_get_brightness());  // Backlight on after all init
}


void engine_display_gc9107_update(uint16_t *screen_buffer_to_render){
    if(dma_channel_is_busy(dma_tx)){
        ENGINE_WARNING_PRINTF("Waiting on previous DMA transfer to complete. Could have done more last frame!");
        dma_channel_wait_for_finish_blocking(dma_tx);
    }

    // For SPI must also wait for FIFO to flush and reset format
    // https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/Processors/TFT_eSPI_RP2040.c#L600-L602
    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    hw_write_masked(&spi_get_hw(spi0)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);

    // Point DMA to active screen buffer that should be
    // sent now that the last frame is finished sending
    txbuf = screen_buffer_to_render;

    gc9107_reset_window();

    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 0);
    gpio_put(PIN_GP16__TO__DC,          1);

    // dma_channel_set_read_addr(dma_tx, txbuf, true);
    dma_channel_configure(dma_tx, &dma_config,
                          &spi_get_hw(spi0)->dr,        // write address
                          txbuf,                        // read address
                          SCREEN_BUFFER_SIZE_PIXELS,    // element count (each element is of size DMA_SIZE_16)
                          true);                        // don't start yet, need to set active frame buffer later
}
