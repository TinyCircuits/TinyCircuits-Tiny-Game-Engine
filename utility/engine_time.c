#include "engine_time.h"
#include <stddef.h>

#ifdef __unix__
    #include <sys/time.h>
#else
    // Last paragraph on page 9: https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf
    #include "pico/stdlib.h"
    #include "pico.h"
    #include "hardware/timer.h"
    #include "pico/time.h"
#endif


uint32_t millis(){
    #ifdef __unix__
        // https://stackoverflow.com/questions/3756323/how-to-get-the-current-time-in-milliseconds-from-c-in-linux
        struct timeval te;
        gettimeofday(&te, NULL);
        uint32_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
        return milliseconds;
    #else
        // https://forums.raspberrypi.com/viewtopic.php?p=1817771&sid=94dff9d898f94703c9600e5378c19561#p1817771
        // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#rpip44f7b6a2c93b1f2927cd
        // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#gab12467c48bde27171b552ac4dc8c7d59
        return to_ms_since_boot(get_absolute_time());
    #endif
}