#if defined(__arm__)
    #include "engine_audio_rp3.h"

    // Pin for PWM audio sample wrap callback (faster than repeating timer, by a lot)
    uint audio_callback_pwm_pin_slice;
    pwm_config audio_callback_pwm_pin_config;


    // Samples each channel, adds, normalizes, and sets PWM
    void ENGINE_FAST_FUNCTION(repeating_audio_callback)(void){


    // Adjusts PWM wrap value based on clock speed
    // to keep callback sample rate consistent
    void engine_audio_rp3_adjust_freq(uint32_t core_clock_hz){
        pwm_config_set_wrap(&audio_callback_pwm_pin_config, (uint16_t)((float)(core_clock_hz) / ENGINE_AUDIO_SAMPLE_RATE) - 1);
    }


    void engine_audio_rp3_init(){
        //generate the interrupt at the audio sample rate to set the PWM duty cycle
        audio_callback_pwm_pin_slice = pwm_gpio_to_slice_num(AUDIO_CALLBACK_PWM_PIN);
        pwm_clear_irq(audio_callback_pwm_pin_slice);
        pwm_set_irq_enabled(audio_callback_pwm_pin_slice, true);
        irq_set_exclusive_handler(PWM_IRQ_WRAP_0, repeating_audio_callback);
        irq_set_priority(PWM_IRQ_WRAP_0, 1);
        irq_set_enabled(PWM_IRQ_WRAP_0, true);
        audio_callback_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_callback_pwm_pin_config, 1);
        engine_audio_rp3_adjust_freq(150 * 1000 * 1000);
        pwm_init(audio_callback_pwm_pin_slice, &audio_callback_pwm_pin_config, true);
    }


    void engine_audio_rp3_setup_playback(){
        // Setup amplifier but make sure it is disabled while PWM is being setup
        gpio_init(AUDIO_ENABLE_PIN);
        gpio_set_dir(AUDIO_ENABLE_PIN, GPIO_OUT);
        gpio_put(AUDIO_ENABLE_PIN, 0);

        // Setup PWM audio pin, bit-depth, and frequency. Duty cycle
        // is only adjusted PWM parameter as samples are retrievd from
        // channel sources
        uint audio_pwm_pin_slice = pwm_gpio_to_slice_num(AUDIO_PWM_PIN);
        gpio_set_function(AUDIO_PWM_PIN, GPIO_FUNC_PWM);
        pwm_config audio_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_pwm_pin_config, 1);
        pwm_config_set_wrap(&audio_pwm_pin_config, 512);   // 125MHz / 1024 = 122kHz
        pwm_init(audio_pwm_pin_slice, &audio_pwm_pin_config, true);

        // Now allow sound to play by enabling the amplifier
        gpio_put(AUDIO_ENABLE_PIN, 1);
    }
#endif