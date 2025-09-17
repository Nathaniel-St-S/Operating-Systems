#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

#define interruptFlag 0 //flag to signal interrupt

//interrupt handler
void interruptHandler();

//Checks for if any interrupts are present
void checkForInterrupt();

#endif 