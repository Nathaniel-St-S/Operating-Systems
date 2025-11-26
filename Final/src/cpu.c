#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/isa.h"
#include <stdint.h>
#include <stdio.h>

Cpu THE_CPU;

// Prints the state of the given CPU
static void print_cpu_state() {
  printf("CPU STATE\n");
  printf("PC:  %X\n", HW_REGISTER(PC));
  printf("IR:  %X\n", HW_REGISTER(IR));
  printf("FLAGS:\n");
  printf("  ZERO:      %1d\n", (HW_REGISTER(FLAGS) >> 0) & 1);
  printf("  OVERFLOW:  %1d\n",  (HW_REGISTER(FLAGS) >> 1) & 1);
  printf("  CARRY:     %1d\n\n\n", (HW_REGISTER(FLAGS) >> 2) & 1);
}

void init_cpu()
{
  HW_REGISTER(PC) = TEXT_BASE;
  HW_REGISTER(IR) = ((uint32_t) -1);
  HW_REGISTER(FLAGS) = F_ZERO;
  printf("Initialized the cpu!\n");
  print_cpu_state();
}

// Fetch the next instruction from the given memory and cpu and increments the program counter
void fetch() {
  HW_REGISTER(IR) = read_word(HW_REGISTER(PC));
  HW_REGISTER(PC)++;
}

// Executes the instruction in the given cpu's IR with the given RAM
void execute() {
  uint32_t instruction = HW_REGISTER(IR);

  execute_instruction(instruction);
}

void cpu_run()
{
  int i = 0;
  while(HW_REGISTER(PC) != CPU_HALT)
  {
    printf("=== Cycle %d ===\n", i + 1);
    fetch();
    execute();
    // check_for_interrupt(); //when processes.c is finished i guess we'll need this. probably not.
    print_cpu_state();
    i++;
  }
}

