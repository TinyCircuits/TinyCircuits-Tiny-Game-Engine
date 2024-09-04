#include "debug_print.h"

void print_binary(uint32_t number, uint32_t bit_count){
    while(bit_count != 0){
        bool bit = number & 0x0001;
        ENGINE_PRINTF("%d", bit);
        number = number >> 1;
        bit_count--;
    }
}

bool DEBUG_INFO_ENABLED = false;
bool DEBUG_WARNINGS_ENABLED = false;
bool DEBUG_ERRORS_ENABLED = false;
bool DEBUG_PERFORMANCE_ENABLED = false;
uint32_t engine_performance_timers[5] = {0};