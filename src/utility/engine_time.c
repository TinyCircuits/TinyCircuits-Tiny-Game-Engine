#include "engine_time.h"
#include <stddef.h>

#if defined(__EMSCRIPTEN__)
    #include <sys/time.h>
    struct timeval  tv;
#elif defined(__unix__)
    #include <time.h>
    struct timespec tp;
#elif defined(__arm__)
    // Last paragraph on page 9: https://datasheets.raspberrypi.com/pico/raspberry-pi-pico-c-sdk.pdf
    #include "pico/stdlib.h"
    #include "pico.h"
    #include "hardware/timer.h"
    #include "pico/time.h"

    /* DWT (Data Watchpoint and Trace) registers, only exists on ARM Cortex with a DWT unit */
    #define KIN1_DWT_CONTROL             (*((volatile uint32_t*)0xE0001000))
    /*!< DWT Control register */
    #define KIN1_DWT_CYCCNTENA_BIT       (1UL<<0)
    /*!< CYCCNTENA bit in DWT_CONTROL register */
    #define KIN1_DWT_CYCCNT              (*((volatile uint32_t*)0xE0001004))
    /*!< DWT Cycle Counter register */
    #define KIN1_DEMCR                   (*((volatile uint32_t*)0xE000EDFC))
    /*!< DEMCR: Debug Exception and Monitor Control Register */
    #define KIN1_TRCENA_BIT              (1UL<<24)
    /*!< Trace enable bit in DEMCR register */

    #define KIN1_InitCycleCounter() \
    KIN1_DEMCR |= KIN1_TRCENA_BIT
    /*!< TRCENA: Enable trace and debug block DEMCR (Debug Exception and Monitor Control Register */
    
    #define KIN1_ResetCycleCounter() \
    KIN1_DWT_CYCCNT = 0
    /*!< Reset cycle counter */
    
    #define KIN1_EnableCycleCounter() \
    KIN1_DWT_CONTROL |= KIN1_DWT_CYCCNTENA_BIT
    /*!< Enable cycle counter */
    
    #define KIN1_DisableCycleCounter() \
    KIN1_DWT_CONTROL &= ~KIN1_DWT_CYCCNTENA_BIT
    /*!< Disable cycle counter */
    
    #define KIN1_GetCycleCounter() \
    KIN1_DWT_CYCCNT
    /*!< Read cycle counter register */
#endif


uint32_t millis(){
    #if defined(__EMSCRIPTEN__)
        gettimeofday(&tv, NULL);
        return tv.tv_sec*1000LL + tv.tv_usec/1000;
    #elif defined(__unix__)
        // https://stackoverflow.com/a/63140531
        // Slow? https://github.com/microsoft/WSL/issues/3697#issuecomment-457767554
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
        return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    #elif defined(__arm__)
        // https://forums.raspberrypi.com/viewtopic.php?p=1817771&sid=94dff9d898f94703c9600e5378c19561#p1817771
        // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#rpip44f7b6a2c93b1f2927cd
        // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#gab12467c48bde27171b552ac4dc8c7d59
        return to_ms_since_boot(get_absolute_time());
    #endif
}


void cycles_start(){
    #ifdef __unix__
        // Not implemented
    #else
        // https://mcuoneclipse.com/2017/01/30/cycle-counting-on-arm-cortex-m-with-dwt/
        KIN1_InitCycleCounter(); /* enable DWT hardware */
        KIN1_ResetCycleCounter(); /* reset cycle counter */
        KIN1_EnableCycleCounter(); /* start counting */
    #endif
}


uint32_t cycles_stop(){
    #ifdef __unix__
        // Not implemented
        return 0;
    #else
        // https://mcuoneclipse.com/2017/01/30/cycle-counting-on-arm-cortex-m-with-dwt/
        uint32_t cycles = KIN1_GetCycleCounter(); /* get cycle counter */
        KIN1_DisableCycleCounter(); /* disable counting if not used any more */
        return cycles;
    #endif
}