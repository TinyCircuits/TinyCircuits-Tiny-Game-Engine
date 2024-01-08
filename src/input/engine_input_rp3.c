#include "engine_input_rp3.h"
#include "engine_input_common.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"


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
    BIT_SET(engine_input_pressed_buttons, BUTTON_DPAD_UP, !gpio_get(GPIO_BUTTON_DPAD_UP));
    BIT_SET(engine_input_pressed_buttons, BUTTON_DPAD_LEFT, !gpio_get(GPIO_BUTTON_DPAD_LEFT));
    BIT_SET(engine_input_pressed_buttons, BUTTON_DPAD_DOWN, !gpio_get(GPIO_BUTTON_DPAD_DOWN));
    BIT_SET(engine_input_pressed_buttons, BUTTON_DPAD_RIGHT, !gpio_get(GPIO_BUTTON_DPAD_RIGHT));
    BIT_SET(engine_input_pressed_buttons, BUTTON_A, !gpio_get(GPIO_BUTTON_A));
    BIT_SET(engine_input_pressed_buttons, BUTTON_B, !gpio_get(GPIO_BUTTON_B));
    BIT_SET(engine_input_pressed_buttons, BUTTON_BUMPER_LEFT, !gpio_get(GPIO_BUTTON_BUMPER_LEFT));
    BIT_SET(engine_input_pressed_buttons, BUTTON_BUMPER_RIGHT, !gpio_get(GPIO_BUTTON_BUMPER_RIGHT));
    BIT_SET(engine_input_pressed_buttons, BUTTON_MENU, !gpio_get(GPIO_BUTTON_MENU));
}
