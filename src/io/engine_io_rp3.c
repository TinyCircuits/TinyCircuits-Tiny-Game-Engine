#include "engine_io_rp3.h"
#include "engine_io_button_codes.h"
#include "debug/debug_print.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "math/engine_math.h"
#include <stdbool.h>


#define GPIO_BUTTON_DPAD_UP         1
#define GPIO_BUTTON_DPAD_LEFT       0
#define GPIO_BUTTON_DPAD_DOWN       3
#define GPIO_BUTTON_DPAD_RIGHT      2
#define GPIO_BUTTON_A               21
#define GPIO_BUTTON_B               25
#define GPIO_BUTTON_BUMPER_LEFT     6
#define GPIO_BUTTON_BUMPER_RIGHT    22
#define GPIO_BUTTON_MENU            26
#define GPIO_RUMBLE                 5
#define GPIO_CHARGE_STAT            24
#define GPIO_LED_R                  11
#define GPIO_LED_G                  10
#define GPIO_LED_B                  12


void engine_io_rp3_pwm_setup(uint gpio, uint16_t default_level){
    uint pwm_pin_slice = pwm_gpio_to_slice_num(gpio);
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    pwm_config pwm_pin_config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&pwm_pin_config, 1);
    pwm_config_set_wrap(&pwm_pin_config, 2048);   // 150MHz / 2048 = 73kHz
    pwm_init(pwm_pin_slice, &pwm_pin_config, true);
    pwm_set_gpio_level(gpio, default_level);
}


void engine_io_rp3_setup(){
    ENGINE_PRINTF("EngineInput: Setting up...\n");

    gpio_init(GPIO_BUTTON_DPAD_UP);
    gpio_init(GPIO_BUTTON_DPAD_LEFT);
    gpio_init(GPIO_BUTTON_DPAD_DOWN);
    gpio_init(GPIO_BUTTON_DPAD_RIGHT);
    gpio_init(GPIO_BUTTON_A);
    gpio_init(GPIO_BUTTON_B);
    gpio_init(GPIO_BUTTON_BUMPER_LEFT);
    gpio_init(GPIO_BUTTON_BUMPER_RIGHT);
    gpio_init(GPIO_BUTTON_MENU);
    gpio_init(GPIO_CHARGE_STAT);
    gpio_init(GPIO_LED_R);
    gpio_init(GPIO_LED_G);
    gpio_init(GPIO_LED_B);

    gpio_pull_up(GPIO_BUTTON_DPAD_UP);
    gpio_pull_up(GPIO_BUTTON_DPAD_LEFT);
    gpio_pull_up(GPIO_BUTTON_DPAD_DOWN);
    gpio_pull_up(GPIO_BUTTON_DPAD_RIGHT);
    gpio_pull_up(GPIO_BUTTON_A);
    gpio_pull_up(GPIO_BUTTON_B);
    gpio_pull_up(GPIO_BUTTON_BUMPER_LEFT);
    gpio_pull_up(GPIO_BUTTON_BUMPER_RIGHT);
    gpio_pull_up(GPIO_BUTTON_MENU);

    gpio_set_dir(GPIO_BUTTON_DPAD_UP, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_DPAD_LEFT, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_DPAD_DOWN, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_DPAD_RIGHT, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_A, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_B, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_BUMPER_LEFT, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_BUMPER_RIGHT, GPIO_IN);
    gpio_set_dir(GPIO_BUTTON_MENU, GPIO_IN);

    gpio_set_dir(GPIO_CHARGE_STAT, GPIO_IN);
    gpio_pull_up(GPIO_CHARGE_STAT);

    gpio_set_dir(GPIO_LED_R, GPIO_OUT);
    gpio_set_dir(GPIO_LED_G, GPIO_OUT);
    gpio_set_dir(GPIO_LED_B, GPIO_OUT);

    engine_io_rp3_pwm_setup(GPIO_RUMBLE, 0);
    // engine_io_rp3_pwm_setup(GPIO_LED_R, 2047);
    // engine_io_rp3_pwm_setup(GPIO_LED_G, 2047);
    // engine_io_rp3_pwm_setup(GPIO_LED_B, 2047);
}


uint16_t engine_io_rp3_pressed_buttons(){
    uint16_t pressed = 0;

    if(gpio_get(GPIO_BUTTON_DPAD_UP) == false) pressed |= BUTTON_CODE_DPAD_UP;
    if(gpio_get(GPIO_BUTTON_DPAD_LEFT) == false) pressed |= BUTTON_CODE_DPAD_LEFT;
    if(gpio_get(GPIO_BUTTON_DPAD_DOWN) == false) pressed |= BUTTON_CODE_DPAD_DOWN;
    if(gpio_get(GPIO_BUTTON_DPAD_RIGHT) == false) pressed |= BUTTON_CODE_DPAD_RIGHT;

    if(gpio_get(GPIO_BUTTON_A) == false) pressed |= BUTTON_CODE_A;
    if(gpio_get(GPIO_BUTTON_B) == false) pressed |= BUTTON_CODE_B;

    if(gpio_get(GPIO_BUTTON_BUMPER_LEFT) == false) pressed |= BUTTON_CODE_BUMPER_LEFT;
    if(gpio_get(GPIO_BUTTON_BUMPER_RIGHT) == false) pressed |= BUTTON_CODE_BUMPER_RIGHT;

    if(gpio_get(GPIO_BUTTON_MENU) == false) pressed |= BUTTON_CODE_MENU;

    return pressed;
}


void engine_io_rp3_rumble(float intensity){
    if(intensity <= EPSILON){
        pwm_set_gpio_level(GPIO_RUMBLE, 0);
    }else{
        pwm_set_gpio_level(GPIO_RUMBLE, (uint32_t)engine_math_map_clamp(intensity, 0.0f, 1.0f, 1400.0f, 2047.0f));
    }
}


bool engine_io_rp3_is_charging(){
    // Charge status line from LiIon charger IC is set
    // pulled HIGH by default and is pulled LOW by IC
    return !gpio_get(GPIO_CHARGE_STAT);
}


void engine_io_rp3_set_indicator(bool off){
    gpio_put(GPIO_LED_R, off);
    gpio_put(GPIO_LED_G, off);
    gpio_put(GPIO_LED_B, off);
}