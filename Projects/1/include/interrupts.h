#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "cpu.h"
#include <stdio.h>

#define MAX_INTERRUPTS 128

//#define INTERRUPTFLAG 1 //flag to signal interrupt

typedef enum irq{
    SAY_HI = 0x1,
    SAY_GOODBYE,
    BREAK,
    EOI, //end of interupt, make sure this is the last in the list
}IRQ;

typedef struct {
    Cpu* items;
    int SP;
} stack;

typedef struct {
    IRQ irq;
    int priority;
} Interrupt;

typedef struct {
    Interrupt data[MAX_INTERRUPTS];
    int size;
} InterruptHeap;

//Initialize the interrupt controller
void init_interrupt_controller(void);

//Add an interupt to the queue to be handled
void add_interrupt(IRQ irq, int priority); 

//Checks for if any interrupts are present
void check_for_interrupt(void);

#endif 
