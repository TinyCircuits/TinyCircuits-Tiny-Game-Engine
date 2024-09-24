#include "debug_print.h"
#include <math.h>

void print_binary(uint32_t number, uint32_t bit_count){
    // ex: bit_count = 16 -> 2^16 = 65536 -> 65536/2 = 32768 = 0b1000_0000_0000_0000
    uint32_t mask_bit = (uint32_t)(powf(2.0f, (float)bit_count)/2.0f);

    while(bit_count != 0){
        // Get the left most bit
        bool bit = number & mask_bit;
        ENGINE_INFO_PRINTF("%d", bit);

        // Move the next bit to the left into the mask bit
        number = number << 1;
        bit_count--;
    }
}

bool DEBUG_INFO_ENABLED = false;
bool DEBUG_WARNINGS_ENABLED = false;
bool DEBUG_ERRORS_ENABLED = false;
bool DEBUG_PERFORMANCE_ENABLED = false;
uint32_t engine_performance_timers[5] = {0};