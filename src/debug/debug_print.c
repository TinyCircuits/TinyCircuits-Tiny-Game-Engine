#include "debug_print.h"

bool DEBUG_INFO_ENABLED = false;
bool DEBUG_WARNINGS_ENABLED = false;
bool DEBUG_ERRORS_ENABLED = false;
bool DEBUG_PERFORMANCE_ENABLED = false;
uint32_t engine_performance_timers[5] = {0};