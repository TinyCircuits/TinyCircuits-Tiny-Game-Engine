#include "engine_fault_rp3.h"
#include "engine_fault_report.h"

#include "hardware/exception.h"


/* The fault handler implementation calls a function called rvGetRegistersFromStack(). */
// https://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html#:~:text=no%20function%20entry-,prologue%20code),-.
// https://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html
// https://forums.raspberrypi.com/viewtopic.php?t=372153#p2228489
// https://github.com/Memotech-Bill/PicoBB/blob/master/src/pico/stack_trap.c
// https://forums.raspberrypi.com/viewtopic.php?p=1896381#p1896381
// https://www.reddit.com/r/arm/comments/142y3ro/exception_handling_in_arm_cortex_m4/
// https://wbk.one/%2Farticle%2F6%2Fdebugging-arm-without-a-debugger-3-printing-stack-trace
static void __attribute__((naked)) engine_fault_get_stack_rp3(void){
    __asm volatile
    (
        " tst lr, #4                                            \n"
        " ite eq                                                \n"
        " mrseq r0, msp                                         \n"
        " mrsne r0, psp                                         \n"
        " ldr r1, [r0, #24]                                     \n"
        " ldr r2, handler2_address_const                        \n"
        " bx r2                                                 \n"
        " handler2_address_const: .word engine_fault_handle_rp3 \n"
    );
}


// https://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html#:~:text=the%20debugger%27s%20CPU-,register%20window,-.
void engine_fault_handle_rp3(uint32_t *pul_fault_stack_address){
    // pg. 32: arm_cortex_m33_trm_100230_0100_07_en.pdf
    // Get PC and make digits into char buffer and print it

    // https://wbk.one/%2Farticle%2F6%2Fdebugging-arm-without-a-debugger-3-printing-stack-trace#:~:text=pc%20(program%20counter)
    uint32_t lr = pul_fault_stack_address[5];   // <- address to return when the current function returns
    uint32_t pc = pul_fault_stack_address[6];   // <- address of the entry point of the function plus 16 bytes
    engine_fault_report(lr, pc);

    // Loop forever
    for(;;);
}


void engine_fault_register_rp3(){
    exception_set_exclusive_handler(HARDFAULT_EXCEPTION, engine_fault_get_stack_rp3);
}