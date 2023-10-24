#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <stdint.h>

#include "py/runtime.h"
#include "py/builtin.h"



#define DEBUG_PRINT_LEVEL_NONE 0        // do not show any debug prints
#define DEBUG_PRINT_LEVEL_ALL 1         // errors, warnings, and info
#define DEBUG_PRINT_LEVEL_WARNINGS 2    // warnings and info
#define DEBUG_PRINT_LEVEL_INFO 3        // info

// The global print level that can be set at runtime
extern uint8_t ENGINE_DEBUG_PRINT_LEVEL;


#define ENGINE_ERROR_PRINTF(fmt, ...)                           \
    if(ENGINE_DEBUG_PRINT_LEVEL == DEBUG_PRINT_LEVEL_ALL &&     \
       ENGINE_DEBUG_PRINT_LEVEL != DEBUG_PRINT_LEVEL_NONE){     \
        mp_printf(&mp_sys_stdout_print, "\x1b[31mERROR: ");     \
        mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);    \
        mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");           \
    }


#define ENGINE_WARNING_PRINTF(fmt, ...)                             \
    if((ENGINE_DEBUG_PRINT_LEVEL == DEBUG_PRINT_LEVEL_ALL ||        \
        ENGINE_DEBUG_PRINT_LEVEL == DEBUG_PRINT_LEVEL_WARNINGS) &&  \
        ENGINE_DEBUG_PRINT_LEVEL != DEBUG_PRINT_LEVEL_NONE){        \
        mp_printf(&mp_sys_stdout_print, "\x1b[33mWARNING: ");       \
        mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);        \
        mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");               \
    }

#define ENGINE_INFO_PRINTF(fmt, ...)                                \
    if((ENGINE_DEBUG_PRINT_LEVEL == DEBUG_PRINT_LEVEL_ALL ||        \
        ENGINE_DEBUG_PRINT_LEVEL == DEBUG_PRINT_LEVEL_WARNINGS ||   \
        ENGINE_DEBUG_PRINT_LEVEL == DEBUG_PRINT_LEVEL_INFO) &&      \
        ENGINE_DEBUG_PRINT_LEVEL != DEBUG_PRINT_LEVEL_NONE){        \
        mp_printf(&mp_sys_stdout_print, "\x1b[32mINFO: ");          \
        mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);        \
        mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");               \
    }

// DO NOT USE: only for special cases. For example, when the debug level
// is changed it will always print a message (changing debug level shouldn't
// be done in games anyway, ugly)
#define ENGINE_FORCE_PRINTF(fmt, ...)                       \
    mp_printf(&mp_sys_stdout_print, "\x1b[35mDEBUG: ");     \
    mp_printf(&mp_sys_stdout_print, fmt, ##__VA_ARGS__);    \
    mp_printf(&mp_sys_stdout_print, "\x1b[0m\n");


#endif  // DEBUG_PRINT_H
