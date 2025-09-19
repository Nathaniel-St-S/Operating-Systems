#include "../include/interrupts.h"
#include "../include/memory.h"
#include "../include/cpu.h"

stack callstack;
InterruptHeap* intrptqueue;

//interrupt handler
void interrupt_handler(Interrupt intrpt) {
    //push the current CPU state to stack
    Cpu init_cpu_state = CPU;
    callstack.items[callstack.SP] = init_cpu_state;
    callstack.SP++;

    //decode the given interupt and handle it
    switch(intrpt.irq) {
        case SAY_HI : printf("hello"); break;
        case SAY_GOODBYE : printf("goodbye"); break;
        case EOI : CPU.flags.INTERRUPT = UNSET_FLAG; break;
    }

    //decrement the CPU stack
    callstack.SP--;
    //reset the CPU to it's original state
    CPU = init_cpu_state;
    //increment the PC to start normal execution
    CPU.PC++;
}

Interrupt curr_intrrpt = NULL;
//Checks for if any interrupts are present
void check_for_interrupt() {
    if (CPU.flags.INTERRUPT) {
        if(&curr_intrrpt == NULL){
            curr_intrrpt = heap_extract_min(intrptqueue);
        }
        Interrupt intrpt = heap_extract_min(intrptqueue);
        if (curr_intrrpt.priority < intrpt.priority) {
            interrupt_handler(curr_intrrpt);
        } else {
            interrupt_handler(intrpt);
        }
    }
}

