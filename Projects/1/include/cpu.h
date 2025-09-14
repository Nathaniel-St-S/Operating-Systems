#ifndef CPU_H
#define CPU_H

#include "types.h"

typedef struct flags {
  int ZERO;
  int CARRY;
  int OVERFLOW;
} FLAGS;

typedef struct cpu {
  word PC;
  word ACC;
  instr IR;
  FLAGS flags;
} CPU;

void cpu_run(CPU* cpu, int program_size, word* data_mem);

void cpu_print_state(const CPU* cpu);

#endif
