#include "engine_display_driver_rp2_gc9107.h"
#include "engine_display_common.h"
#include "debug/debug_print.h"

#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"


// Based on max from Bodmer: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/User_Setup.h#L366
#define GC9107_SPI_MHZ 80 * 1000 * 1000


/* ##### PIN CONNECTIONS #####

    | BRD/PROCESSOR |              GC9107 0.85"              |
    |---------------|-----------------------------------------|
    | VCC+          | VCC [Power 3.3V/5V]                     |
    | GND           | GND [Ground]                            |
    | GP19/SPI0 TX  | SDA [SPI data input/SDA]                |
    | GP18/SPI0 SCK | SCK [SPI clock input]                   |
    | GP17/SPI0 CSn | CS  [Chip select, low=active]           |
    | GP16          | DC  [Data or CMD select, h=data, l=cmd] |
    | GP11          | RST [Reset, low=active/reset]           |
    | GP20          | BL/LCD_LED  [Backlight enable]          |
    |---------------|-----------------------------------------|

    NOTE: Not using PIO since we're using a standard SPI port (see: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/User_Setup.h#L331C90-L332C1)
*/
#define PIN_GP19_SPI0_TX__TO__DIN  PICO_DEFAULT_SPI_TX_PIN      // Default refers to spi port 0
#define PIN_GP18_SPI0_SCK__TO__CLK PICO_DEFAULT_SPI_SCK_PIN     // Default refers to spi port 0
#define PIN_GP17_SPI0_CSn__TO__CS  PICO_DEFAULT_SPI_CSN_PIN     // Default refers to spi port 0
#define PIN_GP20__TO__DC           16
#define PIN_GP21__TO__RST          11
#define PIN_GP22__TO__BL           20

int dma_tx;
dma_channel_config dma_config;
uint16_t *txbuf = NULL;


static void gc9107_write_cmd(uint8_t cmd, const uint8_t* data, size_t length){
    // Interesting note on performance: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/TFT_eSPI.cpp#L3416-L3417
    // Set to 8-bit SPI during cmd sending
    spi_set_format(spi0, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    // Select screen chip and put into cmd mode (see pin connection reference)
    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 0);
    gpio_put(PIN_GP20__TO__DC, 0);

    // Write the command to the driver through SPI
    spi_write_blocking(spi0, &cmd, sizeof(cmd));
    gpio_put(PIN_GP20__TO__DC, 1);

    // If there's also data, write that but put chip in data mode (see pin connection reference)
    if(length > 0){
        spi_write_blocking(spi0, data, length);
    }

    // Now that everything is sent, make sure we're in data mode (and that the screen is not selected? idk)
    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 1);

    // Interesting note on performance: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/TFT_eSPI.cpp#L3416-L3417
    // Set back to 16-bit SPI
    spi_set_format(spi0, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}


void gc9107_set_window(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
    gc9107_write_cmd(0x2a, (uint8_t[]){ x1+2, x2+2 }, 2);
    gc9107_write_cmd(0x2b, (uint8_t[]){ y1+2, y2+2 }, 2);
    gc9107_write_cmd(0x2c, NULL, 0);
}


void engine_display_gc9107_init(){
    ENGINE_INFO_PRINTF("Setting up GC9107 screen");

    // Init SPI
    ENGINE_INFO_PRINTF("Enabling SPI");
    spi_init(spi0, GC9107_SPI_MHZ);

    // Init pins (some are controlled through SPI peripheral
    // and some are controlled through code using GPIO)
    ENGINE_INFO_PRINTF("Enabling pins");
    gpio_set_function(PIN_GP19_SPI0_TX__TO__DIN, GPIO_FUNC_SPI);
    gpio_set_function(PIN_GP18_SPI0_SCK__TO__CLK, GPIO_FUNC_SPI);

    // Setup GPIO
    gpio_init(PIN_GP17_SPI0_CSn__TO__CS);               // Although part of the SPI port, we'll control chip select manually
    gpio_init(PIN_GP20__TO__DC);
    gpio_init(PIN_GP21__TO__RST);
    gpio_init(PIN_GP22__TO__BL);
    gpio_set_dir(PIN_GP17_SPI0_CSn__TO__CS, GPIO_OUT);
    gpio_set_dir(PIN_GP20__TO__DC, GPIO_OUT);
    gpio_set_dir(PIN_GP21__TO__RST, GPIO_OUT);
    gpio_set_dir(PIN_GP22__TO__BL, GPIO_OUT);

    // Do the init sequence
    gpio_put(PIN_GP22__TO__BL, 0);  // Backlight off during init

    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 1);
    sleep_ms(5);
    gpio_put(PIN_GP21__TO__RST, 0);
    sleep_ms(50);
    gpio_put(PIN_GP21__TO__RST, 1);
    sleep_ms(120);

    gc9107_write_cmd(0xB0, (uint8_t[]){ 0xC0 }, 1);
    gc9107_write_cmd(0xB2, (uint8_t[]){ 0x2F }, 1);
    gc9107_write_cmd(0xB3, (uint8_t[]){ 0x03 }, 1);
    gc9107_write_cmd(0xB6, (uint8_t[]){ 0x19 }, 1);
    gc9107_write_cmd(0xB7, (uint8_t[]){ 0x01 }, 1);

    gc9107_write_cmd(0xAC, (uint8_t[]){ 0xCB }, 1);
    gc9107_write_cmd(0xAB, (uint8_t[]){ 0x0e }, 1);

    gc9107_write_cmd(0xB4, (uint8_t[]){ 0x04 }, 1);

    gc9107_write_cmd(0xA8, (uint8_t[]){ 0x19 }, 1);

    gc9107_write_cmd(0x3A, (uint8_t[]){ 0x05 }, 1);

    gc9107_write_cmd(0xb8, (uint8_t[]){ 0x08 }, 1);

    gc9107_write_cmd(0xE8, (uint8_t[]){ 0x24 }, 1);

    gc9107_write_cmd(0xE9, (uint8_t[]){ 0x48 }, 1);

    gc9107_write_cmd(0xea, (uint8_t[]){ 0x22 }, 1);

    gc9107_write_cmd(0xC6, (uint8_t[]){ 0x30 }, 1);
    gc9107_write_cmd(0xC7, (uint8_t[]){ 0x18 }, 1);

    gc9107_write_cmd(0xF0, (uint8_t[]){ 0x1F, 0x28, 0x04, 0x3E, 0x2A, 0x2E, 0x20, 0x00, 0x0C, 0x06, 0x00, 0x1C, 0x1F, 0x0f }, 14);

    gc9107_write_cmd(0xF1, (uint8_t[]){ 0X00, 0X2D, 0X2F, 0X3C, 0X6F, 0X1C, 0X0B, 0X00, 0X00, 0X00, 0X07, 0X0D, 0X11, 0X0f }, 14);

    gc9107_write_cmd(0x21, NULL, 0);

    gc9107_write_cmd(0x11, NULL, 0);
    sleep_ms(120);
    gc9107_write_cmd(0x29, NULL, 0);
    sleep_ms(10);

    // Grab unused dma channel for SPI TX
    ENGINE_INFO_PRINTF("Enabling DMA for 16-bit transfers");
    dma_tx = dma_claim_unused_channel(true);

    // Configure the DMA channel (for SPI TX)
    dma_config = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_dreq(&dma_config, DREQ_SPI0_TX);

    gpio_put(PIN_GP22__TO__BL, 1);  // Backlight on after all init
}


void engine_display_gc9107_update(uint16_t *screen_buffer_to_render){
    if(dma_channel_is_busy(dma_tx)){
        ENGINE_WARNING_PRINTF("Waiting on previous DMA transfer to complete. Could have done more last frame!");
        ENGINE_PERFORMANCE_START(ENGINE_PERF_TIMER_2);
        dma_channel_wait_for_finish_blocking(dma_tx);
        ENGINE_PERFORMANCE_STOP(ENGINE_PERF_TIMER_2, "Time spent waiting on remaining DMA");
    }

    // For SPI must also wait for FIFO to flush and reset format
    // https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/Processors/TFT_eSPI_RP2040.c#L600-L602
    while (spi_get_hw(spi0)->sr & SPI_SSPSR_BSY_BITS) {};
    hw_write_masked(&spi_get_hw(spi0)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);

    // Point DMA to active screen buffer that should be
    // sent now that the last frame is finished sending
    txbuf = screen_buffer_to_render;

    gc9107_set_window(0, 0, 127, 127);

    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 0);
    gpio_put(PIN_GP20__TO__DC,          1);

    // dma_channel_set_read_addr(dma_tx, txbuf, true);
    dma_channel_configure(dma_tx, &dma_config,
                          &spi_get_hw(spi0)->dr,        // write address
                          txbuf,                        // read address
                          SCREEN_BUFFER_SIZE_PIXELS,    // element count (each element is of size DMA_SIZE_16)
                          true);                        // don't start yet, need to set active frame buffer later
}
