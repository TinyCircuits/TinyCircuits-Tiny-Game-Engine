#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdint.h>

#include "py/runtime.h"
#include "py/builtin.h"
#include "engine_time.h"

// These are used for exposing constants the user can use tn enable specific prints
#define DEBUG_SETTING_INFO          0
#define DEBUG_SETTING_WARNINGS      1
#define DEBUG_SETTING_ERRORS        2
#define DEBUG_SETTING_PERFORMANCE   3

extern bool DEBUG_INFO_ENABLED;
extern bool DEBUG_WARNINGS_ENABLED;
extern bool DEBUG_ERRORS_ENABLED;
extern bool DEBUG_PERFORMANCE_ENABLED;

#define ENGINE_PERF_TIMER_1 0
#define ENGINE_PERF_TIMER_2 1
#define ENGINE_PERF_TIMER_3 2
#define ENGINE_PERF_TIMER_4 3
#define ENGINE_PERF_TIMER_5 4
extern uint32_t engine_performance_timers[5];

#define ENGINE_INFO_PRINTF(fmt, ...)                                \
    if(DEBUG_INFO_ENABLED){        \
        mp_printf(&mp_sys_stdout_print, "\x1b[32mINFO: ");          \
        mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);        \
        mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");               \
    }


#define ENGINE_WARNING_PRINTF(fmt, ...)                             \
    if(DEBUG_WARNINGS_ENABLED){        \
        mp_printf(&mp_sys_stdout_print, "\x1b[33mWARNING: ");       \
        mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);        \
        mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");               \
    }



#define ENGINE_ERROR_PRINTF(fmt, ...)                           \
    if(DEBUG_ERRORS_ENABLED){     \
        mp_printf(&mp_sys_stdout_print, "\x1b[31mERROR: ");     \
        mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);    \
        mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");           \
    }


// DO NOT USE: only for special cases. For example, when the debug level
// is changed it will always print a message (changing debug level shouldn't
// be done in games anyway, ugly)
#define ENGINE_FORCE_PRINTF(fmt, ...)                       \
    mp_printf(&mp_sys_stdout_print, "\x1b[35mDEBUG: ");     \
    mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);    \
    mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");


// Tracks time (in ms) when called to used when calling 'ENGINE_PERFORMANCE_STOP()' later
#define ENGINE_PERFORMANCE_START(timer)                                                         \
    mp_printf(&mp_sys_stdout_print, "\x1b[36mPERFORMANCE[timer %d, start]\x1b[0m\n", timer);    \
    engine_performance_timers[(uint8_t)timer] = millis();


// Prints the time (in ms) since 'ENGINE_PERFORMANCE_START()' was called
#define ENGINE_PERFORMANCE_STOP(timer, label)                                                                                                                     \
    if(DEBUG_PERFORMANCE_ENABLED){                                                                                                                                \
        mp_printf(&mp_sys_stdout_print, "\x1b[36mPERFORMANCE[timer %d, end]: %s: %lu ms\x1b[0m\n", (uint8_t)timer, label, millis()-engine_performance_timers[timer]);  \
    }


#endif  // DEBUG_PRINT_H
