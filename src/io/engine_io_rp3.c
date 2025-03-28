#include "engine_io_rp3.h"
#include "engine_io_button_codes.h"
#include "debug/debug_print.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/structs/systick.h"
#include "hardware/exception.h"
#include "hardware/adc.h"
#include "math/engine_math.h"
#include "draw/engine_color.h"
#include "engine_io_module.h"

#include <stdbool.h>

#define CALLBACK_TIMER_ALARM_NUM 1
#define CALLBACK_TIMER_ALARM_IRQ timer_hardware_alarm_get_irq_num(timer_hw, CALLBACK_TIMER_ALARM_NUM)
const uint32_t check_period_delay_us = 1000000; // 1s


// Prototype
void engine_io_rp3_set_timer();

// Used for when the indicator state is set true again, need
// a color to go back to. Set to white by default
volatile uint16_t last_indicator_color_value = 0b0000011111100000;

// When the user sets the state or color of the indicator LED,
// this gets set true;
volatile bool indicator_overridden = false;

// pico-sdk timer for calling battery monitor/indicator updater
repeating_timer_t battery_monitor_cb_timer;

// Use a running average to calculate interpolation
// value for front indicator
#define BATTERY_LEVEL_SAMPLE_COUNT 3
volatile uint8_t battery_level_running_index = 0;
volatile float battery_level_samples[BATTERY_LEVEL_SAMPLE_COUNT];

// Used to limit rate at which battery monitor callback is run even more
volatile uint8_t timer_counter = 0;

void engine_io_rp3_pwm_setup(uint gpio, uint16_t wrap){
    uint pwm_pin_slice = pwm_gpio_to_slice_num(gpio);
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    pwm_config pwm_pin_config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&pwm_pin_config, 1);
    pwm_config_set_wrap(&pwm_pin_config, wrap);   // 150MHz / 2048 = 73kHz, 150MHz / 4092 = 37kHz
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
    if(indicator_overridden){
        return;
    }

    // Make indicator cyan if charging
    if(engine_io_rp3_is_charging()){
        engine_io_rp3_set_indicator_color(0x07FF);
        return;
    }

    // Otherwise, interpolate color from green to red
    // as the battery dies. Get the newest level reading
    float level = 1.0f - engine_io_raw_battery_level();

    // Add level reading to running avg samples
    battery_level_samples[battery_level_running_index] = level;

    // Loop when reach end
    battery_level_running_index++;
    if(battery_level_running_index >= BATTERY_LEVEL_SAMPLE_COUNT){
        battery_level_running_index = 0;
    }

    // Calculate the average
    float level_avg = 0.0f;
    for(uint8_t i=0; i<BATTERY_LEVEL_SAMPLE_COUNT; i++){
        level_avg += battery_level_samples[i];
    }
    level_avg = level_avg / (float)BATTERY_LEVEL_SAMPLE_COUNT;
    
    // Use the average for the color
    uint16_t color = engine_color_blend(0b0000011111100000, 0b1111100000000000, level_avg);

    engine_io_rp3_set_indicator_color(color);
}


void engine_io_rp3_setup(){
    ENGINE_PRINTF("EngineInput: Setting up...\n");

    // Have to set this up now since used right away (
    // unlike other pins)
    gpio_init(GPIO_CHARGE_STAT);
    gpio_set_dir(GPIO_CHARGE_STAT, GPIO_IN);
    gpio_pull_up(GPIO_CHARGE_STAT);

    engine_io_rp3_pwm_setup(GPIO_PWM_RUMBLE, 4096);
    engine_io_rp3_pwm_setup(GPIO_PWM_LED_R, 2048);
    engine_io_rp3_pwm_setup(GPIO_PWM_LED_G, 2048);
    engine_io_rp3_pwm_setup(GPIO_PWM_LED_B, 2048);

    // Battery ADC reading init
    adc_init();
    adc_gpio_init(BATTERY_ADC_GPIO_PIN);
    adc_select_input(BATTERY_ADC_PORT);
    sleep_us(50);   // Delay needed so lagging voltage readings are not taken

    // Fill samples for average level with initial samples
    for(uint8_t i=0; i<BATTERY_LEVEL_SAMPLE_COUNT; i++){
        battery_level_samples[i] = 1.0f - engine_io_raw_battery_level();
        
        // Space out the samples a little bit over time.
        // Needs to be fast and the number of samples small
        // otherwise get blink when soft resets
        sleep_us(50);
    }

    // Update with battery level right away
    engine_io_rp3_update_indicator_level();

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


void engine_io_rp3_reset(){
    // Reset these when engine_main is imported
    indicator_overridden = false;
}


void repeating_battery_monitor_callback(){
    if(timer_counter > 24){
        engine_io_rp3_update_indicator_level();
        timer_counter = 0;
    }
    
    timer_counter++;
    
    pwm_clear_irq(PWM_BATTERY_MONITOR_TIMER_SLICE_NUM);
}


void engine_io_rp3_set_timer(){
    // Start the main ENGINE_AUDIO_SAMPLE_RATE (22050Hz) audio sample rate playback interrupt
    //
    // Switched from pico-sdk repeating timer because of:
    //  * https://github.com/raspberrypi/pico-sdk/issues/2118
    //  * https://github.com/raspberrypi/pico-sdk/releases/tag/2.1.1#:~:text=to%20increase%20performance-,pico_time,-Fixed%20a%20rare
    pwm_clear_irq(PWM_BATTERY_MONITOR_TIMER_SLICE_NUM);
    pwm_set_irq_enabled(PWM_BATTERY_MONITOR_TIMER_SLICE_NUM, true);
    irq_add_shared_handler(PWM_IRQ_WRAP, repeating_battery_monitor_callback, 0);
    irq_set_priority(PWM_IRQ_WRAP, 1);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&config, 255);
    pwm_config_set_wrap(&config, UINT16_MAX);
    pwm_init(PWM_BATTERY_MONITOR_TIMER_SLICE_NUM, &config, true);
}


void engine_io_rp3_battery_monitor_setup(){
    // Check battery and update front indicator every second
    engine_io_rp3_set_timer();
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
        pwm_set_gpio_level(GPIO_PWM_RUMBLE, (uint32_t)engine_math_map_clamp(intensity, 0.0f, 1.0f, 2800.0f, 4095.0f));
    }
}


bool engine_io_rp3_is_charging(){
    // Charge status line from LiIon charger IC is set
    // pulled HIGH by default and is pulled LOW by IC
    return !gpio_get(GPIO_CHARGE_STAT);
}