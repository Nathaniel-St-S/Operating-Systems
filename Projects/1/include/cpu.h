#ifndef CPU_H
#define CPU_H

#include "types.h"
#include "isa.h"

#define EMPTY_REG ((word) - 1)
#define UNSET_FLAG ((int) -1)

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
extern Cpu CPU;

//initialize a CPU to fetch, decode, and execute instructions
static void init_cpu(Cpu* cpu);

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
static void set_zero_flag(word value);

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
void set_add_flags(word a, word b, word r);

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
void set_sub_flags(word a, word b, word r);

// Fetch the next instruction from the given memory and cpu
// and increments the program counter
static void fetch(void);

// Decodes the given instruction into its operator and operand
static Decoded decode(instr instruction);

// Executes the instruction in the given cpu's IR with the given RAM
static void execute(void);

// Runs the fetch-execution cycle program_size times or until a halt is encountered
static void cpu_run(int program_size, word* mem);

// Prints the state of the given CPU
static void cpu_print_state(void);

#endif
