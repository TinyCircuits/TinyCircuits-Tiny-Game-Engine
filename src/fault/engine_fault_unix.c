#include "engine_fault_unix.h"
#include "engine_fault_report.h"

#include "debug/debug_print.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// https://stackoverflow.com/a/2436368
void engine_fault_handle_unix(int signal, siginfo_t *si, void *arg){
    engine_fault_report(0, 0);

    // Loop forever
    for(;;);
}


void engine_fault_register_unix(){
    struct sigaction action;
    action.sa_sigaction = &engine_fault_handle_unix;
    action.sa_flags = SA_SIGINFO;

    sigaction(11, &action, NULL);
}