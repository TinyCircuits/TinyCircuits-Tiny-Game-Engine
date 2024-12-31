#include "engine_io_rp3.h"
#include "engine_io_button_codes.h"
#include "debug/debug_print.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "engine_io_module.h"

#include <stdbool.h>


// Used for when the indicator state is set true again, need
// a color to go back to. Set to white by default
uint16_t last_indicator_color_value = 0xffff;

// When the user sets the state or color of the indicator LED,
// this gets set true;
bool indicator_overridden = false;

// pico-sdk timer for calling battery monitor/indicator updater
repeating_timer_t battery_monitor_cb_timer;


void engine_io_rp3_pwm_setup(uint gpio){
    uint pwm_pin_slice = pwm_gpio_to_slice_num(gpio);
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    pwm_config pwm_pin_config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&pwm_pin_config, 1);
    pwm_config_set_wrap(&pwm_pin_config, 2048);   // 150MHz / 2048 = 73kHz
    pwm_init(pwm_pin_slice, &pwm_pin_config, true);
    pwm_set_gpio_level(gpio, 0);
}


void engine_io_rp3_set_indicator_overridden(bool overridden){
    indicator_overridden = overridden;
}


void engine_io_rp3_set_indicator_color(uint16_t color){
    // TODO: Might be able to use something like this: https://github.com/gpshead/pwm_lightness
    float r_percent = 1.0f - engine_color_get_r_float(color);
    float g_percent = 1.0f - engine_color_get_g_float(color);
    float b_percent = 1.0f - engine_color_get_b_float(color);

    pwm_set_gpio_level(GPIO_PWM_LED_R, (uint16_t)(r_percent * 2047.0f));
    pwm_set_gpio_level(GPIO_PWM_LED_G, (uint16_t)(g_percent * 2047.0f));
    pwm_set_gpio_level(GPIO_PWM_LED_B, (uint16_t)(b_percent * 2047.0f));

    // Save the last value for when indicator might be
    // reenabled in the future
    last_indicator_color_value = color;
}


void engine_io_rp3_set_indicator_state(bool on){
    if(on){
        engine_io_rp3_set_indicator_color(last_indicator_color_value);
    }else{
        pwm_set_gpio_level(GPIO_PWM_LED_R, 2047);
        pwm_set_gpio_level(GPIO_PWM_LED_G, 2047);
        pwm_set_gpio_level(GPIO_PWM_LED_B, 2047);
    }
}


// Updates the front indicator LED based on
// battery level
void engine_io_rp3_update_indicator_level(){
    // Don't do anything if the user did
    // anything to the indicator
    if(indicator_overriden){
        return;
    }

    // Make indicator cyan if charging
    if(engine_io_rp3_is_charging()){
        engine_io_rp3_set_indicator_color(0x07FF);
        return;
    }

    // Otherwise, interpolate color from green to red
    // as the battery dies
    float level = 1.0f - engine_io_raw_battery_level();
    
    uint16_t color = engine_color_blend(0b0000011111100000, 0b1111100000000000, level);

    engine_io_rp3_set_indicator_color(color);
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

    engine_io_rp3_pwm_setup(GPIO_PWM_RUMBLE);
    engine_io_rp3_pwm_setup(GPIO_PWM_LED_R);
    engine_io_rp3_pwm_setup(GPIO_PWM_LED_G);
    engine_io_rp3_pwm_setup(GPIO_PWM_LED_B);

    // Battery ADC reading init
    adc_init();
    adc_gpio_init(BATTERY_ADC_GPIO_PIN);
    adc_select_input(BATTERY_ADC_PORT);

    // Update with battery level right away
    engine_io_rp3_update_indicator_level();
}


void engine_io_rp3_reset(){
    // Reset these when engine_main is imported
    indicator_overriden = false;
}


bool repeating_battery_monitor_callback(repeating_timer_t *rt){
    engine_io_rp3_update_indicator_level();
    return true;
}


void engine_io_rp3_battery_monitor_setup(){
    // Check battery and update front indicator every second
    add_repeating_timer_ms(-1000, &repeating_battery_monitor_callback, NULL, &battery_monitor_cb_timer);
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
        pwm_set_gpio_level(GPIO_PWM_RUMBLE, 0);
    }else{
        pwm_set_gpio_level(GPIO_PWM_RUMBLE, (uint32_t)engine_math_map_clamp(intensity, 0.0f, 1.0f, 1400.0f, 2047.0f));
    }
}


bool engine_io_rp3_is_charging(){
    // Charge status line from LiIon charger IC is set
    // pulled HIGH by default and is pulled LOW by IC
    return !gpio_get(GPIO_CHARGE_STAT);
}