#ifndef ENGINE_DEFINES_H
#define ENGINE_DEFINES_H

#if defined(__unix__)
    #define ENGINE_FAST_FUNCTION(x) inline x
#elif defined(__arm__)
    #define ENGINE_FAST_FUNCTION(x) __time_critical_func(x) // https://www.raspberrypi.com/documentation/pico-sdk/runtime.html#ga27ef91000958320e25ff481d16786ebf
#else
    #warning "Unknown platform for defining fast functions"
    #define ENGINE_FAST_FUNCTION(x) x
#endif

#endif