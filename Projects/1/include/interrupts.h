#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

#define INTERRUPTFLAG 1 //flag to signal interrupt

//interrupt handler
void interrupt_handler(void);

//Checks for if any interrupts are present
void check_for_interrupt(void);

#endif 