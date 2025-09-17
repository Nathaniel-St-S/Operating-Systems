#include "../include/interrupts.h"
#include "../include/cpu.h"


//interrupt handler
void interruptHandler() {
    Cpu currentCpuState = CPU;

}

//Checks for if any interrupts are present
void checkForInterrupt() {
    if (interruptFlag != 0) {
        interruptHandler();
    }
}

