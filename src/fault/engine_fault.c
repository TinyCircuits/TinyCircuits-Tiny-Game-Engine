#include "engine_fault_rp3.h"
#include "engine_fault_unix.h"


void engine_fault_handling_register(){
    #if defined(__arm__)
        engine_fault_register_rp3();
    #elif defined(__unix__)
        engine_fault_register_unix();
    #else
        #error "EngineFaultHandling: ERROR: Registering fault handler on this platform is not implemented!"
    #endif
}