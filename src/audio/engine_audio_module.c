#include "py/obj.h"
#include "engine_audio_module.h"
#include "debug/debug_print.h"
#include <stdlib.h>

// The number of total audio channels that
// can be active at a single time
#define CHANNEL_COUNT 8

// The total number of bytes dedicated to storing audio
// can be calculated as:
//      CHANNEL_COUNT * CHANNEL_BUFFER_SIZE * 2 (2 since each channel has dual buffers)
//      8 * 256 * 2 = 4096 bytes
// This buffer needs to large enough such that the target sample rate doesn't
// rip through it before DMA can fill the other buffer. For example, if the
// target sample rate is 44100Hz and samples take 2 bytes then it takes:
//
// (1/44100) * (CHANNEL_BUFFER_SIZE / BYTES_PER_SAMPLE)
// (1/44100) * (256/2) = 2.9ms
//
// to get through the buffer. Hopefully DMA can take CHANNEL_BUFFER_SIZE bytes out of flash
// and into RAM faster than that
#define CHANNEL_BUFFER_SIZE 256

#if defined(__unix__)
    // Nothing to do
#elif defined(__arm__)
    #include "pico/stdlib.h"
    #include "hardware/dma.h"

    // pg. 542: https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
    // https://github.com/raspberrypi/pico-examples/blob/master/timer/hello_timer/hello_timer.c#L11-L57
    // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#rpipdb65a0bdce0635d95877
    // https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#interrupt-numbers
    struct repeating_timer repeating_audio_timer;

    // Samples each channel, adds, normalizes, and sets PWM
    bool repeating_audio_callback(struct repeating_timer *t) {
        // pwm_set_gpio_level(512);

        // On each sample interrupt, go through each channel source
        // and get the next sample. The sample from each source
        // will contribute to the current PWM amplitude.
        // For each channel, if the number of samples consumed
        // equals the total number of samples available in the buffer,
        // switch the 'read' and 'fill' buffer indices and start filling
        // the buffer that is now 'fill' with new data from the channel source

        // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/dma/hello_dma/hello_dma.c#L32-L39
        // for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        //     dma_channel_configure(
        //         channels[icx].dma_channel,  // Channel to be configured
        //         &channels[icx].dma_config,  // The configuration we just created
        //         dst,   // dst: The initial write address
        //         src,                        // src: The initial read address
        //         CHANNEL_BUFFER_SIZE,        // Number of transfers; in this case each is 1 byte.
        //         true                        // Start immediately.
        //     );
        // }

        return true;
    }
#endif


typedef struct{
    mp_obj_t channel_source;        // Can be WaveSoundResource, etc. (sources keep track of position and end)
    bool looping;                   // If true, the 'channel_source' is never set to mp_const_none
    int dma_channel;                // DMA channel used for pulling audio data from flash into one of two dual buffers
    dma_channel_config dma_config;  // DMA channel config for pull data from flash
    uint8_t *buffers[2];            // Dual buffers for this channel
    uint8_t read_buffer_index;      // Index of the current buffer in 'buffers' to read and play samples from;
    uint8_t fill_buffer_index;      // Index of the current buffer in 'buffers' to fill with future data from flash
}audio_channel_t;


// 8 audio channels. Since audio is large it has to stay in
// flash, but flash is slow to get data from which is a problem
// when sample retrieval time/latency needs to be small to keep
// on track at the relatively high sample rate good audio requires.
// DMA is used to copy data into one of the dual buffer pairs
// each audio channel gets but there's only 12 channels on RP2040
// and one is used for the screen
audio_channel_t channels[CHANNEL_COUNT];


void engine_audio_setup(){
    #if defined(__unix__)
        // Nothing to do
    #elif defined(__arm__)
        // Setup amplifier but make sure it is disabled while PWM is being setup
        gpio_init(26);
        gpio_set_dir(26, GPIO_OUT);
        gpio_put(26, 0);

        // Setup PWM audio pin, bit-depth, and frequency. Duty cycle
        // is only adjusted PWM parameter as samples are retrievd from
        // channel sources
        uint audio_pwm_pin_slice = pwm_gpio_to_slice_num(23);
        gpio_set_function(23, GPIO_FUNC_PWM);
        pwm_config audio_pwm_pin_config = pwm_get_default_config();
        pwm_config_set_clkdiv_int(&audio_pwm_pin_config, 1);
        pwm_config_set_wrap(&audio_pwm_pin_config, 1024);   // 125MHz / 1024 = 122kHz
        pwm_init(audio_pwm_pin_slice, &audio_pwm_pin_config, true);

        // Now allow sound to play by enabling the amplifier
        gpio_put(26, 1);

        // Setup timer ISR. Set sampling rate to 44100
        add_repeating_timer_us((int64_t)((1.0/44100.0) * 1000000.0), repeating_audio_callback, NULL, &repeating_audio_timer);
    #endif

    // Setup audio buffers. Should make sure this doesn't happen every engine_init(): TODO
    for(uint8_t icx=0; icx<CHANNEL_COUNT; icx++){
        channels[icx].looping = false;

        // Use C heap to store these persistent buffers
        channels[icx].buffers[0] = malloc(sizeof(uint8_t) * CHANNEL_BUFFER_SIZE);
        channels[icx].buffers[1] = malloc(sizeof(uint8_t) * CHANNEL_BUFFER_SIZE);

        // https://github.com/raspberrypi/pico-examples/blob/eca13acf57916a0bd5961028314006983894fc84/dma/hello_dma/hello_dma.c#L27-L30
        channels[icx].dma_channel = dma_claim_unused_channel(true);

        channels[icx].dma_config = dma_channel_get_default_config(channels[icx].dma_channel);
        channel_config_set_transfer_data_size(&channels[icx].dma_config, DMA_SIZE_8);           // Samples aren't always 16-bits, could be 8-bit
        channel_config_set_read_increment(&channels[icx].dma_config, true);                     // Automaticaly increment read position
        channel_config_set_write_increment(&channels[icx].dma_config, true);                    // Automaticaly increment write position
    }
}


STATIC mp_obj_t engine_audio_play(mp_obj_t sound_source_obj, mp_obj_t channel_index_obj, mp_obj_t looping_obj){
    // Should probably make sure this doesn't interfere with DMA or interrupt: TODO
    uint8_t channel_index = mp_obj_get_int(channel_index_obj);
    channels[channel_index].channel_source = sound_source_obj;
    channels[channel_index].looping = mp_obj_get_int(looping_obj);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(engine_audio_play_obj, engine_audio_play);


// Module attributes
STATIC const mp_rom_map_elem_t engine_audio_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_engine_audio) },
};

// Module init
STATIC MP_DEFINE_CONST_DICT (mp_module_engine_audio_globals, engine_audio_globals_table);

const mp_obj_module_t engine_audio_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_engine_audio_globals,
};

MP_REGISTER_MODULE(MP_QSTR_engine_audio, engine_audio_user_cmodule);