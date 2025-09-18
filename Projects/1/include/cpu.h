#ifndef CPU_H
#define CPU_H

#include "isa.h"

#define EMPTY_REG ((word) - 1)
#define UNSET_FLAG ((int) - 1)

typedef struct {
  int ZERO;
  int CARRY;
  int OVERFLOW;
  int INTERRUPT;
} Flags;

typedef struct {
  // word EAX;
  // word EBX;
  // word ECX;
  // word EDX;
  word PC;
  word ACC;
  word IR;
  Flags flags;
} Cpu;

/**
 * Represents the lower 4 and upper 12 bits from a instruction.
 */
typedef struct {
  OP       op;
  mem_addr addr;
} Decoded;

//CPU to control execution
extern Cpu CPU;

//initialize a CPU to fetch, decode, and execute instructions
void init_cpu(Cpu* cpu);

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
void set_zero_flag(word value);

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
void set_add_flags(word a, word b, word r);

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
void set_sub_flags(word a, word b, word r);

// Fetch the next instruction from the given memory and cpu
// and increments the program counter
void fetch(void);

// Decodes the given instruction into its operator and operand
Decoded decode(word instruction);

// Executes the instruction in the given cpu's IR with the given RAM
void execute(void);

// Runs the fetch-execution cycle program_size times or until a halt is encountered
void cpu_run(int program_size, word* mem);

// Prints the state of the given CPU
void cpu_print_state(void);

#endif
