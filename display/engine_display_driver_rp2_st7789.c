#include "engine_display_driver_rp2_st7789.h"
#include "engine_display_common.h"
#include "../utility/debug_print.h"

#include <string.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "extmod/machine_spi.h"

// Based on max from Bodmer: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/User_Setup.h#L366
#define ST7789_SPI_MHZ 80 * 1000 * 1000


/* ##### PIN CONNECTIONS #####

    Pico:      pg. 5: https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf
    Pico W:    pg. 6: https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf
    Waveshare:        https://www.waveshare.com/1.54inch-LCD-Module.htm#:~:text=Power%20(3.3V/5V%20input)

    |        PICO        |         WAVESHARE ST7789 1.54"          |
    |--------------------|-----------------------------------------|
    | 3v3(OUT)      [36] | VCC [Power 3.3V/5V]                     |
    | GND           [38] | GND [Ground]                            |
    | GP19/SPI0 TX  [25] | DIN [SPI data input/SDA]                |
    | GP18/SPI0 SCK [24] | CLK [SPI clock input]                   |
    | GP17/SPI0 CSn [22] | CS  [Chip select, low=active]           |
    | GP20          [26] | DC  [Data or CMD select, h=data, l=cmd] |
    | GP21          [27] | RST [Reset, low=active/reset]           |
    | GP22          [29] | BL  [Backlight enable]                  |
    |--------------------|-----------------------------------------|

    NOTE: Not using PIO since we're using a standard SPI port (see: https://github.com/Bodmer/TFT_eSPI/blob/5162af0a0e13e0d4bc0e4c792ed28d38599a1f23/User_Setup.h#L331C90-L332C1)
*/
#define PIN_GP19_SPI0_TX__TO__DIN  PICO_DEFAULT_SPI_TX_PIN      // Default refers to spi port 0
#define PIN_GP18_SPI0_SCK__TO__CLK PICO_DEFAULT_SPI_SCK_PIN     // Default refers to spi port 0
#define PIN_GP17_SPI0_CSn__TO__CS  PICO_DEFAULT_SPI_CSN_PIN     // Default refers to spi port 0
#define PIN_GP20__TO__DC           20
#define PIN_GP21__TO__RST          21
#define PIN_GP22__TO__BL           22


int dma_tx;
dma_channel_config dma_config;
uint16_t *txbuf = NULL;


const uint16_t window_x0 = (240/2) - (SCREEN_WIDTH/2);
const uint16_t window_x1 = (240/2) + (SCREEN_WIDTH/2) - 1;  // Very important that the address window is the correct SIZE!

const uint16_t window_y0 = (240/2) - (SCREEN_HEIGHT/2);
const uint16_t window_y1 = (240/2) + (SCREEN_HEIGHT/2) - 1; // Very important that the address window is the correct SIZE!

// Default window based on: https://github.com/ArmDeveloperEcosystem/st7789-library-for-pico/blob/8e652102388b3592244119bfa24013ec42e5a7ed/src/st7789.c#L53-L77
static uint8_t column_address_data[] = {
    window_x0 >> 8,
    window_x0 & 0xff,
    window_x1 >> 8,
    window_x1 & 0xff,
};
static uint8_t row_address_data[] = {
    window_y0 >> 8,
    window_y0 & 0xff,
    window_y1 >> 8,
    window_y1 & 0xff,
};


static void st7789_write_cmd(uint8_t cmd, const uint8_t* data, size_t length){
    // https://github.com/ArmDeveloperEcosystem/st7789-library-for-pico/blob/8e652102388b3592244119bfa24013ec42e5a7ed/src/st7789.c#L21
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

    spi_set_format(spi0, 16, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);
}


static void st7789_reset_window(){
    // CASET (2Ah): Column Address Set
    st7789_write_cmd(0x2a, column_address_data, sizeof(column_address_data));

    // RASET (2Bh): Row Address Set
    st7789_write_cmd(0x2b, row_address_data, sizeof(row_address_data));

    // RAMWR
    st7789_write_cmd(0x2C, NULL, 0);
}


static void st7789_init_cmd_sequence(){
    ENGINE_INFO_PRINTF("Sending screen init commands...");

    // Screen init cmd sequence: ref: https://github.com/ArmDeveloperEcosystem/st7789-library-for-pico/blob/8e652102388b3592244119bfa24013ec42e5a7ed/src/st7789.c#L116-L154
    gpio_put(PIN_GP22__TO__BL, 0);  // Backlight off during init

    // SWRESET (01h): Software Reset
    st7789_write_cmd(0x01, NULL, 0);
    sleep_ms(150);

    // SLPOUT (11h): Sleep Out
    st7789_write_cmd(0x11, NULL, 0);
    sleep_ms(50);

    // COLMOD (3Ah): Interface Pixel Format
    // - RGB interface color format     = 65K of RGB interface
    // - Control interface color format = 16bit/pixel
    st7789_write_cmd(0x3a, (uint8_t[]){ 0x55 }, 1);
    sleep_ms(10);

    // MADCTL (36h): Memory Data Access Control
    // - Page Address Order            = Top to Bottom
    // - Column Address Order          = Left to Right
    // - Page/Column Order             = Normal Mode
    // - Line Address Order            = LCD Refresh Top to Bottom
    // - RGB/BGR Order                 = RGB
    // - Display Data Latch Data Order = LCD Refresh Left to Right
    st7789_write_cmd(0x36, (uint8_t[]){ 0x00 }, 1);

    // INVON (21h): Display Inversion On
    st7789_write_cmd(0x21, NULL, 0);
    sleep_ms(10);

    // NORON (13h): Normal Display Mode On
    st7789_write_cmd(0x13, NULL, 0);
    sleep_ms(10);

    // DISPON (29h): Display On
    st7789_write_cmd(0x29, NULL, 0);
    sleep_ms(10);

    gpio_put(PIN_GP22__TO__BL, 1);  // Backlight on after init

    ENGINE_INFO_PRINTF("Done sending screen init commands!");
}


void engine_display_st7789_init(){
    ENGINE_INFO_PRINTF("Setting up ST7899 screen");

    // Init SPI
    ENGINE_INFO_PRINTF("Enabling SPI");
    spi_init(spi0, ST7789_SPI_MHZ);

    // Init pins (some are controlled through SPI peripheral 
    // and some are controlled through code using GPIO)
    ENGINE_INFO_PRINTF("Enabling pins");
    gpio_set_function(PIN_GP19_SPI0_TX__TO__DIN, GPIO_FUNC_SPI);
    gpio_set_function(PIN_GP18_SPI0_SCK__TO__CLK, GPIO_FUNC_SPI);

    // Reference: https://github.com/ArmDeveloperEcosystem/st7789-library-for-pico/blob/8e652102388b3592244119bfa24013ec42e5a7ed/src/st7789.c#L102
    gpio_init(PIN_GP17_SPI0_CSn__TO__CS);               // Although part of the SPI port, we'll control chip select manually
    gpio_init(PIN_GP20__TO__DC);
    gpio_init(PIN_GP21__TO__RST);
    gpio_init(PIN_GP22__TO__BL);
    gpio_set_dir(PIN_GP17_SPI0_CSn__TO__CS, GPIO_OUT);
    gpio_set_dir(PIN_GP20__TO__DC, GPIO_OUT);
    gpio_set_dir(PIN_GP21__TO__RST, GPIO_OUT);
    gpio_set_dir(PIN_GP22__TO__BL, GPIO_OUT);

    // Make sure not in reset
    gpio_put(PIN_GP21__TO__RST, 1);
    sleep_us(1);

    // Send each init command
    st7789_init_cmd_sequence();

    // Grab unused dma channel for SPI TX
    ENGINE_INFO_PRINTF("Enabling DMA for 16-bit transfers");
    dma_tx = dma_claim_unused_channel(true);

    // Configure the DMA channel (for SPI TX)
    dma_config = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_16);
    channel_config_set_dreq(&dma_config, DREQ_SPI0_TX);
}


void engine_display_st7789_update(uint16_t *screen_buffer_to_render){
    // Point DMA to active screen buffer that should be
    // sent once the last frame is finished sending
    txbuf = screen_buffer_to_render;

    if(dma_channel_is_busy(dma_tx)){
        ENGINE_WARNING_PRINTF("Waiting on previous DMA transfer to complete. Could have done more last frame!");
        ENGINE_PERFORMANCE_START(ENGINE_PERF_TIMER_2);
        dma_channel_wait_for_finish_blocking(dma_tx);
        ENGINE_PERFORMANCE_STOP(ENGINE_PERF_TIMER_2, "Time spent waiting on remaining DMA");
    }

    st7789_reset_window();

    gpio_put(PIN_GP17_SPI0_CSn__TO__CS, 0);
    gpio_put(PIN_GP20__TO__DC,          1);

    // dma_channel_set_read_addr(dma_tx, txbuf, true);
    dma_channel_configure(dma_tx, &dma_config,
                          &spi_get_hw(spi0)->dr, // write address
                          txbuf,                // read address
                          SCREEN_BUFFER_SIZE_PIXELS,   // element count (each element is of size DMA_SIZE_16)
                          true);               // don't start yet, need to set active frame buffer later
}
