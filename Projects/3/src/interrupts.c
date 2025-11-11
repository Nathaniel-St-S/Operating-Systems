#include "../include/interrupts.h"

InterruptHeap INTERRUPTCONTROLLER;
#define INTBLOCK(location) INTERRUPTCONTROLLER.data[location]
stack callstack;
Interrupt curr_intrrpt;

/* --------------------- Stack Helpers --------------------- */

void push_cpu_state(Cpu cpu)
{
  if (callstack.SP >= CALLSTACK_SIZE)
  {
    fprintf(stderr, "Call stack overflow!\n");
    return;
  }
  callstack.items[callstack.SP++] = cpu;
}

Cpu pop_cpu_state()
{
  if (callstack.SP <= 0)
  {
    fprintf(stderr, "Call stack underflow!\n");
    Cpu empty = {0};
    return empty;
  }
  return callstack.items[--callstack.SP];
}

/* --------------------- Heap Helpers --------------------- */

static void swap(Interrupt* a, Interrupt* b)
{
  Interrupt tmp = *a;
  *a = *b;
  *b = tmp;
}

static int parent(int i) { return (i - 1) / 2; }
static int left(int i)   { return 2 * i + 1; }
static int right(int i)  { return 2 * i + 2; }

/* --------------------- Core Functions --------------------- */

void init_interrupt_controller(void)
{
  INTERRUPTCONTROLLER.size = 0;

  for (int i = 0; i < MAX_INTERRUPTS; i++) {
    INTBLOCK(i).irq = EOI;
    INTBLOCK(i).priority = 100000;
  }

  callstack.SP = 0;
  callstack.items = malloc(sizeof(Cpu) * CALLSTACK_SIZE);
  if (!callstack.items)
  {
    fprintf(stderr, "Failed to allocate callstack\n");
    exit(EXIT_FAILURE);
  }

  curr_intrrpt.irq = EOI;
  curr_intrrpt.priority = 100000;

  printf("Initialized interrupt controller.\n");
}

// No more memory leaks yay :)
void free_interrupt_controller(void) {
    free(callstack.items);
    callstack.items = NULL;
}

// Add an interrupt to the heap (lower number = higher priority)
void add_interrupt(IRQ irq, int priority)
{
  if (INTERRUPTCONTROLLER.size >= MAX_INTERRUPTS)
  {
    fprintf(stderr, "Interrupt queue full!\n");
    return;
  }

  int i = INTERRUPTCONTROLLER.size++;
  INTBLOCK(i).irq = irq;
  INTBLOCK(i).priority = priority;

    
  while (i != 0 && INTBLOCK(parent(i)).priority > INTBLOCK(i).priority)
  {
    swap(&INTBLOCK(i), &INTBLOCK(parent(i)));
    i = parent(i);
  }

  printf("[INTERRUPT] Queued IRQ %d (priority %d)\n", irq, priority);
}

/* Pop the highest-priority interrupt */
static Interrupt next_interrupt(void)
{
  if (INTERRUPTCONTROLLER.size <= 0)
  {
  Interrupt none = { EOI, 100000 };
  return none;
  }

  Interrupt root = INTBLOCK(0);
  INTBLOCK(0) = INTBLOCK(--INTERRUPTCONTROLLER.size);

  int i = 0;
  while (1)
  {
    int l = left(i), r = right(i), smallest = i;

    if (l < INTERRUPTCONTROLLER.size &&
            INTBLOCK(l).priority < INTBLOCK(smallest).priority)
            smallest = l;

    if (r < INTERRUPTCONTROLLER.size &&
            INTBLOCK(r).priority < INTBLOCK(smallest).priority)
            smallest = r;

    if (smallest != i)
    {
      swap(&INTBLOCK(i), &INTBLOCK(smallest));
      i = smallest;
    }
    else break;
  }

  return root;
}

/* Handle a single interrupt */
static void interrupt_handler(Interrupt intrpt) {
  // Save CPU state
  push_cpu_state(THE_CPU);

  switch (intrpt.irq)
  {
    case SAY_HI:
      printf("[INTERRUPT HANDLER] Hello from IRQ %d!\n", intrpt.irq);
      break;
    case SAY_GOODBYE:
      printf("[INTERRUPT HANDLER] Goodbye from IRQ %d!\n", intrpt.irq);
      break;
    case EOI:
      // End of interrupt, do nothing
      break;
    default:
      fprintf(stderr, "[INTERRUPT ERROR] Unknown IRQ %d\n", intrpt.irq);
      break;
  }

  // Restore CPU state
  THE_CPU = pop_cpu_state();
}

void check_for_interrupt(void)
{
  if (INTERRUPTCONTROLLER.size == 0){return;}

  Interrupt intrpt = next_interrupt();
  curr_intrrpt = intrpt;

  printf("[INTERRUPT DISPATCH] Handling IRQ %d (priority %d)\n", intrpt.irq, intrpt.priority);

  interrupt_handler(intrpt);
}

