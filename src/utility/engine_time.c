#include "engine_time.h"
#include "debug/debug_print.h"
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

uint32_t millis_internal(){
    #if defined(__EMSCRIPTEN__)
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
    #elif defined(__unix__)
        // https://stackoverflow.com/a/63140531
        // Slow? https://github.com/microsoft/WSL/issues/3697#issuecomment-457767554
        // Use real clock time so that the time progresses also when sleeping.
        // An alternative is CLOCK_PROCESS_CPUTIME_ID, which only measures CPU time of this process.
        clock_gettime(CLOCK_REALTIME, &tp);
        return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
    #elif defined(__arm__)
        // https://forums.raspberrypi.com/viewtopic.php?p=1817771&sid=94dff9d898f94703c9600e5378c19561#p1817771
        // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#rpip44f7b6a2c93b1f2927cd
        // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#gab12467c48bde27171b552ac4dc8c7d59
        return to_ms_since_boot(get_absolute_time());
    #endif
}


// Returns an increasing millisecond counter with an arbitrary reference point, that wraps around
// after some value.
//
// The wrap-around value is not explicitly exposed, but we will refer to it as TICKS_MAX to
// simplify discussion. Period of the values is TICKS_PERIOD = TICKS_MAX + 1. TICKS_PERIOD is
// guaranteed to be a power of two, but otherwise no assumptions about it should be made.
// Thus, this function will return a value in range [0 .. TICKS_MAX], inclusive, total TICKS_PERIOD
// values. Note that only non-negative values are used. For the most part, you should treat values
// returned by these functions as opaque. The only operations available for them are
//  millis_diff() and ticks_add() functions described below.
//
// Note: Performing standard mathematical operations (+, -) or relational operators (<, <=, >, >=)
// directly on this value will lead to invalid result. Performing mathematical operations and then
// passing their results as arguments to millis_diff() or millis_add() will also lead to invalid
// results from the latter functions.
//
// This is based on the contract and implementation of MicroPython's time.ticks_ms() function:
// https://docs.micropython.org/en/latest/library/time.html#time.ticks_ms
uint32_t millis() {
    return millis_internal() & (MILLIS_PERIOD - 1);
}


// Returns the difference between two values returned by millis() function.
//
// The contract and implementation is based on MicroPython's time.ticks_diff() function:
// https://docs.micropython.org/en/latest/library/time.html#time.ticks_diff
int32_t millis_diff(uint32_t end, uint32_t start) {
    return ((end - start + MILLIS_PERIOD / 2) & (MILLIS_PERIOD - 1)) - MILLIS_PERIOD / 2;
}


// Returns the sum of a value returned by millis() function and an integer delta.
//
// The contract and implementation is based on MicroPython's time.ticks_add() function:
// https://docs.micropython.org/en/latest/library/time.html#time.ticks_add
uint32_t millis_add(uint32_t millis, int32_t delta) {
    if (delta + MILLIS_PERIOD / 2 - 1 >= MILLIS_PERIOD - 1) {
        ENGINE_ERROR_PRINTF("Millis delta is %d, out of range", delta);
    }
    return ((millis + delta) & (MILLIS_PERIOD - 1));
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