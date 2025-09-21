#include "../include/interrupts.h"

InterruptHeap INTERRUPTCONTROLLER;
stack callstack;

void swap(Interrupt a, Interrupt b) {
    Interrupt tmp = a;
    a = b;
    b = tmp;
}

int parent(int i) { return (i - 1) / 2; }
int left(int i)   { return 2 * i + 1; }
int right(int i)  { return 2 * i + 2; }

void add_interrupt(IRQ irq, int priority) {
    if (INTERRUPTCONTROLLER.size >= MAX_INTERRUPTS) {
        printf("Heap overflow!\n");
        return;
    }

    // Insert new interrupt at end
    int i = INTERRUPTCONTROLLER.size++;
    INTERRUPTCONTROLLER.data[i].irq = irq;
    INTERRUPTCONTROLLER.data[i].priority = priority;

    // reoder the interrupts by priority
    // higher priority interrupts have a higher number 
    while (i != 0 && INTERRUPTCONTROLLER.data[parent(i)].priority > INTERRUPTCONTROLLER.data[i].priority) {
        swap(INTERRUPTCONTROLLER.data[i], INTERRUPTCONTROLLER.data[parent(i)]);
        i = parent(i);
    }

    //Set the interrupt flag to let the cpu know about an incoming interrupt
    CPU.flags.INTERRUPT = irq;
}

Interrupt next_interrupt() {
    if (INTERRUPTCONTROLLER.size <= 0) {
        printf("Heap underflow!\n");
        return (Interrupt){-1, -1};
    }

    Interrupt root = INTERRUPTCONTROLLER.data[0];
    INTERRUPTCONTROLLER.data[0] = INTERRUPTCONTROLLER.data[--INTERRUPTCONTROLLER.size];

    // Heapify down
    int i = 0;
    while (1) {
        int l = left(i), r = right(i), smallest = i;

        if (l < INTERRUPTCONTROLLER.size && INTERRUPTCONTROLLER.data[l].priority < INTERRUPTCONTROLLER.data[smallest].priority)
            smallest = l;
        if (r < INTERRUPTCONTROLLER.size && INTERRUPTCONTROLLER.data[r].priority < INTERRUPTCONTROLLER.data[smallest].priority)
            smallest = r;

        if (smallest != i) {
            swap(INTERRUPTCONTROLLER.data[i], INTERRUPTCONTROLLER.data[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return root;
}

void init_interrupt_controller(){
  INTERRUPTCONTROLLER.size = 0;
    for(int i = 0; i < MAX_INTERRUPTS; i++){
      INTERRUPTCONTROLLER.data[i].irq = -1;
      INTERRUPTCONTROLLER.data[i].priority = 100000;
  }
  printf("initialized interrupt controller\n");
}

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
        default:
          printf("ERROR: Invalid irq -> %u <-\n", (unsigned)intrpt.irq);
          CPU.PC = CPU_HALT;

    }

    //decrement the CPU stack
    callstack.SP--;
    //reset the CPU to it's original state
    CPU = init_cpu_state;
    //increment the PC to start normal execution
    CPU.PC++;
}

Interrupt curr_intrrpt;
//Checks for if any interrupts are present
void check_for_interrupt() {
    printf("got here");
    if (CPU.flags.INTERRUPT) {
        //if(INTERRUPTCONTROLLER.size == 0){
        //    curr_intrrpt = next_interrupt();
        //}
        Interrupt intrpt = next_interrupt();
        if (curr_intrrpt.priority < intrpt.priority) {
            interrupt_handler(curr_intrrpt);
        } else {
            curr_intrrpt = intrpt;
            interrupt_handler(curr_intrrpt);
        }
    }
}

