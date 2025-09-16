#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "isa.h"

#define EMPTY_REG ((word) - 1)
#define UNSET_FLAG INT_MIN

typedef struct {
  int ZERO;
  int CARRY;
  int OVERFLOW;
} Flags;

typedef struct {
  word EAX;
  word EBX;
  word ECX;
  word EDX;
  word PC;
  word ACC;
  instr IR;
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
extern Cpu* CPU;

//initialize a CPU to fetch, decode, and execute instructions
static void init_cpu(Cpu* cpu);

// Fetch the next instruction from the given memory and cpu
// and increments the program counter
static void fetch(word* data_mem);

// Decodes the given instruction into its operator and operand
static Decoded decode(instr instruction);

// Executes the instruction in the given cpu's IR with the given RAM
static void execute(CPU* cpu, word* data_mem);

// Runs the fetch-execution cycle program_size times or until a halt is encountered
static void cpu_run(CPU* cpu, int program_size, word* mem);

// Prints the state of the given CPU
static void cpu_print_state(const CPU* cpu);

#endif
