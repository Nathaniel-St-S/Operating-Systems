#include "../include/interrupts.h"

InterruptHeap INTERRUPTCONTROLLER;
stack callstack;
Interrupt curr_intrrpt;

void swap(Interrupt* a, Interrupt* b) {
    Interrupt tmp = *a;
    *a = *b;
    *b = tmp;
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
        swap(&INTERRUPTCONTROLLER.data[i], &INTERRUPTCONTROLLER.data[parent(i)]);
        i = parent(i);
    }

    //Set the interrupt flag to let the cpu know about an incoming interrupt
    set_interrupt_flag(true);
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
            swap(&INTERRUPTCONTROLLER.data[i], &INTERRUPTCONTROLLER.data[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return root;
}

void reset_curr_interrupt()
{
  //set the curr interupt to it's default state
  curr_intrrpt.irq = -1;
  curr_intrrpt.priority = 100000;
}

void init_interrupt_controller(){
  INTERRUPTCONTROLLER.size = 0;
    for(int i = 0; i < MAX_INTERRUPTS; i++){
      INTERRUPTCONTROLLER.data[i].irq = -1;
      INTERRUPTCONTROLLER.data[i].priority = 100000;
  }
  // initialize callstack
    callstack.SP = 0;
    callstack.items = malloc(sizeof(Cpu) * CALLSTACK_SIZE);
    if (!callstack.items) {
        fprintf(stderr, "Failed to allocate callstack\n");
        exit(1);
    }

    reset_curr_interrupt();

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
    case SAY_HI :
      printf("INTERRUPT: hello\n");
      set_interrupt_flag(false);
      reset_curr_interrupt();
      break;
    case SAY_GOODBYE :
      printf("INTERRUPT: goodbye\n");
      set_interrupt_flag(false);
      reset_curr_interrupt();
      break;
    case EOI : 
      set_interrupt_flag(false); 
      reset_curr_interrupt();
      break;
    default:
      printf("ERROR: Invalid irq -> %u <-\n", (unsigned)intrpt.irq);
      CPU.PC = CPU_HALT;
      break;
    }

    //decrement the CPU stack
    callstack.SP--;
    //reset the CPU to it's original state
    CPU = callstack.items[callstack.SP];
    //increment the PC to start normal execution
    //CPU.PC++;
}

//Checks for if any interrupts are present
void check_for_interrupt() {
  if (!CPU.flags.INTERRUPT) return;
  if (INTERRUPTCONTROLLER.size == 0){
    //no more interrupts in que, so clear flag
    set_interrupt_flag(false);
    return;
  }
    
  Interrupt intrpt = next_interrupt();
  if(curr_intrrpt.irq == -1 || intrpt.priority < curr_intrrpt.priority){
    curr_intrrpt = intrpt;
    interrupt_handler(curr_intrrpt);
  }else{
    interrupt_handler(curr_intrrpt);
    add_interrupt(intrpt.irq, intrpt.priority);
  }

  //clear flag if heap is empty after handle
  if(INTERRUPTCONTROLLER.size == 0) set_interrupt_flag(false);

}
