#include "../include/interrupts.h"
#include "../include/memory.h"
#include "../include/cpu.h"


//interrupt handler
void interrupt_handler() {
    Cpu init_cpu_state = CPU;
    
    while(CPU.flags.INTERRUPT) {
        if(CPU.IR == 0xA) {
            break;
        } else {
            fetch();
            execute();
        }

    }

    Cpu CPU = init_cpu_state;
    //cpu_run(20, RAM);
}

//Checks for if any interrupts are present
void check_for_interrupt() {
    if (INTERRUPTFLAG) {
        interrupt_handler();
    }
}

