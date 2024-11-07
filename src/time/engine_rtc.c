#include "engine_rtc.h"
#include "debug/debug_print.h"
#include <string.h>


#if defined(__arm__)
    #include "../lib/bm8563/bm8563.h"
    #include "pico/stdlib.h"
    #include "hardware/gpio.h"
    #include "hardware/i2c.h"
    #include "pico/time.h"

    bm8563_t bm;

    #define RTC_I2C_SCL_GPIO 9
    #define RTC_I2C_SDA_GPIO 8

    // Custom I2C reader/writer functions for bm8563 library to call
    int32_t custom_i2c_read(void *handle, uint8_t address, uint8_t reg, uint8_t *buffer, uint16_t size){
        // Write register we want to read from
        int ret = i2c_write_timeout_us(i2c0, address, &reg, 1, true, 500000);
        if(ret == PICO_ERROR_GENERIC || ret == PICO_ERROR_TIMEOUT){
            return BM8563_ERROR_NOTTY;
        }

        // Read what was returned by the write
        ret = i2c_read_timeout_us(i2c0, address, buffer, size, false, 500000);
        if(ret == PICO_ERROR_GENERIC || ret == PICO_ERROR_TIMEOUT){
            return BM8563_ERROR_NOTTY;
        }

        return BM8563_OK;
    }

    int32_t custom_i2c_write(void *handle, uint8_t address, uint8_t reg, const uint8_t *buffer, uint16_t size){
        uint8_t data[size+1];
        data[0] = reg;
        memcpy(data+1, buffer, size);

        // Write the data
        int ret = i2c_write_timeout_us(i2c0, address, data, size+1, false, 500000);
        if(ret == PICO_ERROR_GENERIC || ret == PICO_ERROR_TIMEOUT){
            return BM8563_ERROR_NOTTY;
        }

        return BM8563_OK;
    }
#endif


void engine_rtc_init(){
    #if defined(__arm__)
        // Setup I2C instance and pins: https://github.com/micropython/micropython/blob/ba98533454eef5ab5783039f9929351c8f54d005/ports/rp2/machine_i2c.c#L136-L146
        // https://www.lcsc.com/datasheet/lcsc_datasheet_2308181040_GATEMODE-BM8563EMA_C269878.pdf <- actual part 
        // https://static.chipdip.ru/lib/031/DOC043031000.pdf
        // https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf
        i2c_init(i2c0, 100000);  // Reduced from 400kHz to 100kHz because of I2C errors and freezing due to pull up resistor values
        gpio_set_function(RTC_I2C_SCL_GPIO, GPIO_FUNC_I2C);
        gpio_set_function(RTC_I2C_SDA_GPIO, GPIO_FUNC_I2C);
        gpio_set_pulls(RTC_I2C_SCL_GPIO, true, 0);
        gpio_set_pulls(RTC_I2C_SDA_GPIO, true, 0);

        // Setup bm8563 library: https://github.com/tuupola/bm8563
        bm.read = &custom_i2c_read;
        bm.write = &custom_i2c_write;
        bm.handle = NULL;
        bm8563_init(&bm);
    #endif
}


int engine_rtc_check_compromised(){
    #if defined(__arm__)
        struct tm rtc;

        bm8563_err_t status = bm8563_read(&bm, &rtc);

        if(status == BM8563_ERR_LOW_VOLTAGE){
            return true;
        }else if(status == BM8563_ERROR_NOTTY){
            return ENGINE_RTC_I2C_ERROR;
        }

    #endif

    return false;
}


int engine_rtc_get_datetime(struct tm *rtc){
    #if defined(__arm__)
        bm8563_err_t status = bm8563_read(&bm, rtc);

        // If the BM is OK or only low voltage, the time is still OK
        // but if neither, the only option is an I2C error
        if(status == BM8563_ERROR_NOTTY){
            return ENGINE_RTC_I2C_ERROR;
        }
    #elif defined(__unix__)
        time_t t = time(NULL);
        struct tm *lt = localtime(&t);
        memmove(rtc, lt, sizeof(struct tm));
    #endif

    return ENGINE_RTC_OK;
}


int engine_rtc_set_datetime(struct tm *rtc){
    #if defined(__arm__)
        bm8563_err_t status = bm8563_write(&bm, rtc);

        // If the write was not OK, then the only option is an I2C ERROR
        if(status == BM8563_ERROR_NOTTY){
            return ENGINE_RTC_I2C_ERROR;
        }
    #elif defined(__unix__)
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("EngineTime: ERROR: Setting time on UNIX port is not supported"));
    #endif

    return ENGINE_RTC_OK;
}