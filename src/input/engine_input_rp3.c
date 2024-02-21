#include "engine_input_rp3.h"
#include "engine_input_common.h"
#include "debug/debug_print.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdbool.h>


#define GPIO_BUTTON_DPAD_UP         1
#define GPIO_BUTTON_DPAD_LEFT       6 
#define GPIO_BUTTON_DPAD_DOWN       3
#define GPIO_BUTTON_DPAD_RIGHT      2
#define GPIO_BUTTON_A               22
#define GPIO_BUTTON_B               25
#define GPIO_BUTTON_BUMPER_LEFT     7
#define GPIO_BUTTON_BUMPER_RIGHT    21
#define GPIO_BUTTON_MENU            0


void engine_input_rp3_setup(){
    ENGINE_FORCE_PRINTF("EngineInput: Setting up...");

    gpio_init(GPIO_BUTTON_DPAD_UP);
    gpio_init(GPIO_BUTTON_DPAD_LEFT);
    gpio_init(GPIO_BUTTON_DPAD_DOWN);
    gpio_init(GPIO_BUTTON_DPAD_RIGHT);
    gpio_init(GPIO_BUTTON_A);
    gpio_init(GPIO_BUTTON_B);
    gpio_init(GPIO_BUTTON_BUMPER_LEFT);
    gpio_init(GPIO_BUTTON_BUMPER_RIGHT);
    gpio_init(GPIO_BUTTON_MENU);

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
}


void engine_input_rp3_update_pressed_mask(){
    // Reset this to all unpressed before checking if pressed
    engine_input_pressed_buttons = 0;

    if(gpio_get(GPIO_BUTTON_DPAD_UP) == false) engine_input_pressed_buttons |= BUTTON_DPAD_UP;
    if(gpio_get(GPIO_BUTTON_DPAD_LEFT) == false) engine_input_pressed_buttons |= BUTTON_DPAD_LEFT;
    if(gpio_get(GPIO_BUTTON_DPAD_DOWN) == false) engine_input_pressed_buttons |= BUTTON_DPAD_DOWN;
    if(gpio_get(GPIO_BUTTON_DPAD_RIGHT) == false) engine_input_pressed_buttons |= BUTTON_DPAD_RIGHT;

    if(gpio_get(GPIO_BUTTON_A) == false) engine_input_pressed_buttons |= BUTTON_A;
    if(gpio_get(GPIO_BUTTON_B) == false) engine_input_pressed_buttons |= BUTTON_B;

    if(gpio_get(GPIO_BUTTON_BUMPER_LEFT) == false) engine_input_pressed_buttons |= BUTTON_BUMPER_LEFT;
    if(gpio_get(GPIO_BUTTON_BUMPER_RIGHT) == false) engine_input_pressed_buttons |= BUTTON_BUMPER_RIGHT;

    if(gpio_get(GPIO_BUTTON_MENU) == false) engine_input_pressed_buttons |= BUTTON_MENU;
}
