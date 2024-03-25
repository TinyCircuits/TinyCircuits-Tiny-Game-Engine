#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdint.h>

#include "py/runtime.h"
#include "py/builtin.h"
#include "py/mpprint.h"
#include "../utility/engine_time.h"

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
    if(DEBUG_INFO_ENABLED){                                         \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[32mINFO: ");          \
        mp_printf(MP_PYTHON_PRINTER, fmt, ##__VA_ARGS__);        \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[0m\n");               \
    }


#define ENGINE_WARNING_PRINTF(fmt, ...)                             \
    if(DEBUG_WARNINGS_ENABLED){                                     \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[33mWARNING: ");       \
        mp_printf(MP_PYTHON_PRINTER, fmt, ##__VA_ARGS__);        \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[0m\n");               \
    }



#define ENGINE_ERROR_PRINTF(fmt, ...)                           \
    if(DEBUG_ERRORS_ENABLED){                                   \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[31mERROR: ");     \
        mp_printf(MP_PYTHON_PRINTER, fmt, ##__VA_ARGS__);    \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[0m\n");           \
    }


// DO NOT USE: only for special cases. For example, when the debug level
// is changed it will always print a message (changing debug level shouldn't
// be done in games anyway, ugly)
#define ENGINE_FORCE_PRINTF(fmt, ...)                       \
    mp_printf(MP_PYTHON_PRINTER, "\x1b[35mDEBUG: ");     \
    mp_printf(MP_PYTHON_PRINTER, fmt, ##__VA_ARGS__);    \
    mp_printf(MP_PYTHON_PRINTER, "\x1b[0m\n");


#define ENGINE_PRINTF(fmt, ...)                       \
    mp_printf(MP_PYTHON_PRINTER, fmt, ##__VA_ARGS__);    \
    mp_printf(MP_PYTHON_PRINTER, "\n");


// Tracks time (in ms) when called to used when calling 'ENGINE_PERFORMANCE_STOP()' later
#define ENGINE_PERFORMANCE_START(timer)                                                             \
    if(DEBUG_PERFORMANCE_ENABLED){                                                                  \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[36mPERFORMANCE[timer %d, start]\x1b[0m\n", timer+1);  \
        engine_performance_timers[(uint8_t)timer] = millis();                                       \
    }


// Prints the time (in ms) since 'ENGINE_PERFORMANCE_START()' was called
#define ENGINE_PERFORMANCE_STOP(timer, label)                                                                                                   \
    if(DEBUG_PERFORMANCE_ENABLED){                                                                                                              \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[36mPERFORMANCE[timer %d, end]: %s: %lu ms\x1b[0m\n", timer+1, label, millis()-engine_performance_timers[timer]);  \
    }


// Tracks time (in ms) when called to used when calling 'ENGINE_PERFORMANCE_STOP()' later
#define ENGINE_PERFORMANCE_CYCLES_START()                                                   \
    if(DEBUG_PERFORMANCE_ENABLED){                                                          \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[36mPERFORMANCE CYCLES START:\x1b[0m\n");      \
        cycles_start();                                                                     \
    }


// Prints the time (in ms) since 'ENGINE_PERFORMANCE_START()' was called
#define ENGINE_PERFORMANCE_CYCLES_STOP()                                                                         \
    if(DEBUG_PERFORMANCE_ENABLED){                                                                               \
        mp_printf(MP_PYTHON_PRINTER, "\x1b[36mPERFORMANCE CYCLES STOP: %lu cycles\x1b[0m\n", cycles_stop());  \
    }


#endif  // DEBUG_PRINT_H
