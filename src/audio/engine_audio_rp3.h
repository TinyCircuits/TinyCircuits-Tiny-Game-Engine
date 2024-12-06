#ifndef ENGINE_AUDIO_RP3_H
#define ENGINE_AUDIO_RP3_H

#if defined(__arm__)
    #include "pico/stdlib.h"
    #include "hardware/dma.h"
    #include "hardware/structs/xip_ctrl.h"
    #include "hardware/pwm.h"
    #include "hardware/adc.h"
    #include "hardware/timer.h"
    #include "pico/multicore.h"

    #define AUDIO_PWM_PIN 23
    #define AUDIO_CALLBACK_PWM_PIN 24
    #define AUDIO_ENABLE_PIN 20

    void engine_audio_rp3_adjust_freq(uint32_t core_clock_hz);

    void engine_audio_rp3_init_one_time();
    void engine_audio_rp3_channel_init_one_time(int *dma_channel, dma_channel_config *dma_config);
    void engine_audio_rp3_setup_playback();
    void engine_audio_rp3_copy(int dma_copy_channel, dma_channel_config *dma_copy_config, uint8_t *dest, uint8_t *src, uint32_t count);
#endif

#endif  // ENGINE_AUDIO_WEB_H