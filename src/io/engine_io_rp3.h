#ifndef ENGINE_IO_RP3_H
#define ENGINE_IO_RP3_H

#include <stdbool.h>
#include <stdint.h>


#define GPIO_BUTTON_DPAD_UP         1
#define GPIO_BUTTON_DPAD_LEFT       0
#define GPIO_BUTTON_DPAD_DOWN       3
#define GPIO_BUTTON_DPAD_RIGHT      2
#define GPIO_BUTTON_A               21
#define GPIO_BUTTON_B               25
#define GPIO_BUTTON_BUMPER_LEFT     6
#define GPIO_BUTTON_BUMPER_RIGHT    22
#define GPIO_BUTTON_MENU            26
#define GPIO_PWM_RUMBLE             5   // PWM2 B
#define GPIO_CHARGE_STAT            24
#define GPIO_PWM_LED_R              11  // PWM5 B
#define GPIO_PWM_LED_G              10  // PWM5 A
#define GPIO_PWM_LED_B              12  // PWM6 A


#define AUDIO_PWM_PIN 23                // PWM3 B
#define AUDIO_ENABLE_PIN 20             // PWM2 A


/* ##### PIN CONNECTIONS #####

    | BRD/PROCESSOR |              GC9107 0.85"               |
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
#define PIN_GP19_SPI0_TX__TO__SDA  PICO_DEFAULT_SPI_TX_PIN      // Default refers to spi port 0
#define PIN_GP18_SPI0_SCK__TO__CLK PICO_DEFAULT_SPI_SCK_PIN     // Default refers to spi port 0
#define PIN_GP17_SPI0_CSn__TO__CS  PICO_DEFAULT_SPI_CSN_PIN     // Default refers to spi port 0
#define PIN_GP16__TO__DC          16
#define PIN_GP4__TO__RST          4
#define PIN_GP7_PIO_PWM__TO__BL   7     // PWM3 B


void engine_io_rp3_set_indicator_color(uint16_t color);
void engine_io_rp3_set_indicator_state(bool on);

void engine_io_rp3_setup();
void engine_io_rp3_battery_monitor_setup();
uint16_t engine_io_rp3_pressed_buttons();
void engine_io_rp3_rumble(float intensity);
bool engine_io_rp3_is_charging();

#endif // ENGINE_INPUT_RP3_H
