#if defined(__arm__)
    #include "engine_audio_rp3.h"
    #include "audio/engine_audio_module.h"
    #include "utility/engine_defines.h"
    #include "math/engine_math.h"
    #include "debug/debug_print.h"

    // Pin for PWM audio sample wrap callback (faster than repeating timer, by a lot)
    uint audio_callback_pwm_pin_slice;
    pwm_config audio_callback_pwm_pin_config;


    // Samples each channel, adds, normalizes, and sets PWM
    void ENGINE_FAST_FUNCTION(engine_audio_rp3_playback_cb)(void){
        bool play = false;    // Set `true` if at least one channel is ready to play
        float output = engine_audio_get_mixed_output_sample(&play);

        if(play){
            // Up to the user to make sure all playing channels do not add up and
            // go out of -1.0 ~ 1.0 range. Clamp the total sample sum since
            // very likely it could end of out of bounds, and map to PWM levels
            // NOTE: Set PWM wrap to 512 levels so it will use values from 0 to 511
            output = engine_math_map_clamp(output, -1.0f, 1.0f, 0.0f, 511.0f);

            // Actually set the wrap value to play this sample
            pwm_set_gpio_level(AUDIO_PWM_PIN, (uint32_t)(output));
        }

        pwm_clear_irq(audio_callback_pwm_pin_slice);

        return;
    }


    // Adjusts PWM wrap value based on clock speed
    // to keep callback sample rate consistent
    void engine_audio_rp3_adjust_freq(uint32_t core_clock_hz){
        pwm_config_set_wrap(&audio_callback_pwm_pin_config, (uint16_t)((float)(core_clock_hz) / ENGINE_AUDIO_SAMPLE_RATE) - 1);
    }


    void engine_audio_rp3_init_one_time(){
        // Generate the interrupt at the audio sample rate to set the PWM duty cycle
        audio_callback_pwm_pin_slice = pwm_gpio_to_slice_num(AUDIO_CALLBACK_PWM_PIN);
        pwm_clear_irq(audio_callback_pwm_pin_slice);
        pwm_set_irq_enabled(audio_callback_pwm_pin_slice, true);
        irq_set_exclusive_handler(PWM_IRQ_WRAP_0, engine_audio_rp3_playback_cb);
        irq_set_priority(PWM_IRQ_WRAP_0, 1);
        irq_set_enabled(PWM_IRQ_WRAP_0, true);
        audio_callback_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_callback_pwm_pin_config, 1);
        engine_audio_rp3_adjust_freq(150 * 1000 * 1000);
        pwm_init(audio_callback_pwm_pin_slice, &audio_callback_pwm_pin_config, true);
    }


    void engine_audio_rp3_channel_init_one_time(int *dma_channel, dma_channel_config *dma_config){
        *dma_channel = dma_claim_unused_channel(true);
        *dma_config  = dma_channel_get_default_config(*dma_channel);
        channel_config_set_transfer_data_size(dma_config, DMA_SIZE_8);
        channel_config_set_read_increment(dma_config, true);
        channel_config_set_write_increment(dma_config, true);
        // channel_config_set_dreq(&self->dma_config, DREQ_XIP_STREAM); // When this is set DMA never finishes (see pg. 127 of rp2040 datasheet: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
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
        pwm_config_set_wrap(&audio_pwm_pin_config, 512);   // 150MHz / 512 = 146kHz
        pwm_init(audio_pwm_pin_slice, &audio_pwm_pin_config, true);

        // Now allow sound to play by enabling the amplifier
        gpio_put(AUDIO_ENABLE_PIN, 1);
    }


    void engine_audio_rp3_copy(audio_channel_class_obj_t *channel, uint8_t *dest, uint8_t *src, uint32_t count){
        // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/flash/xip_stream/flash_xip_stream.c#L45-L48
        // while (!(xip_ctrl_hw->stat & XIP_STAT_FIFO_EMPTY))
        //     (void) xip_ctrl_hw->stream_fifo;
        // xip_ctrl_hw->stream_addr = (uint32_t)current_source_data;
        // xip_ctrl_hw->stream_ctr = channel->buffer_end;

        if(dma_channel_is_busy(channel->dma_copy_channel)){
            ENGINE_WARNING_PRINTF("AudioModule: Waiting on previous DMA transfer to complete, this ideally shouldn't happen");
            dma_channel_wait_for_finish_blocking(channel->dma_copy_channel);
        }

        // https://github.com/raspberrypi/pico-examples/blob/master/flash/xip_stream/flash_xip_stream.c#L58-L70
        dma_channel_configure(
            channel->dma_copy_channel,   // Channel to be configured
            &channel->dma_copy_config,   // The configuration we just created
            dest,                        // The initial write address
            src,                         // The initial read address
            count,                       // Number of transfers; in this case each is 1 byte
            true                         // Start immediately
        );
    }
#endif