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
    // gpio_init(GPIO_RUMBLE);

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
    // gpio_set_dir(GPIO_RUMBLE, GPIO_OUT);

    uint rumple_pwm_pin_slice = pwm_gpio_to_slice_num(GPIO_RUMBLE);
    gpio_set_function(GPIO_RUMBLE, GPIO_FUNC_PWM);
    pwm_config rumble_pwm_pin_config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&rumble_pwm_pin_config, 1);
    pwm_config_set_wrap(&rumble_pwm_pin_config, 2048);   // 125MHz / 1024 = 122kHz
    pwm_init(rumple_pwm_pin_slice, &rumble_pwm_pin_config, true);
    pwm_set_gpio_level(GPIO_RUMBLE, 0);
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
        pwm_set_gpio_level(GPIO_RUMBLE, (uint32_t)engine_math_map_clamp(intensity, 0.0f, 1.0f, 1400.0f, 2048.0f));
    }
}
