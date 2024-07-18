#ifndef ENGINE_FAULT_REPORT_H
#define ENGINE_FAULT_REPORT_H

#include <stdint.h>

// Generates a report that can be used to
// investigate why the engine crashed
void engine_fault_report(uint32_t pc);

#endif  // ENGINE_FAULT_REPORT_H