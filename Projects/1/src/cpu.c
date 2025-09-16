#include <stdint.h>
#include <stdio.h>
#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"

#define CPU_HALT (word)0xFFFF

/**
 * Enum for the supported operations.
 * This includes loading, storing, adding, subtracting, and halting.
 */
typedef enum op {
  OP_LOAD  = 0x1,
  OP_STORE = 0x2,
  OP_ADD   = 0x5,
  OP_SUB   = 0x6,
  OP_HALT  = 0xF,
} OP;

/**
 * Represents the lower 4 and upper 12 bits from a instruction.
 */
typedef struct {
  OP       op;
  mem_addr addr;
} Decoded;

/* --- helpers ------------------------------------------------------------- */

// Decodes the given instruction into its operator and operand
static inline Decoded decode(instr instruction) {
  Decoded d;
  d.op   = (OP)((instruction & 0xF000u) >> 12);
  d.addr = (mem_addr)(instruction & 0x0FFFu);
  return d;
}

// Sets the zero flag of the given cpu to 1 if the value is 0, 0 otherwise
static inline void set_zero_flag(CPU *cpu, word value) {
  cpu->flags.ZERO = (value == 0);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a + b = r
static inline void set_add_flags(CPU *cpu, word a, word b, word r) {
  // Unsigned carry out of bit 15
  cpu->flags.CARRY =
      ((uint32_t)(uword)a + (uint32_t)(uword)b) > 0xFFFFu;

  // Signed overflow: inputs same sign, result different sign
  cpu->flags.OVERFLOW =
      ((a >= 0 && b >= 0 && r <  0) ||
       (a <  0 && b <  0 && r >= 0));

  set_zero_flag(cpu, r);
}

// Sets the carry, overflow, and zero flags of the given cpu based on the given a - b = r
static inline void set_sub_flags(CPU *cpu, word a, word b, word r) {
  // Borrow in unsigned
  cpu->flags.CARRY = ((uword)a < (uword)b);

  // Signed overflow: inputs different sign, result sign differs from a
  cpu->flags.OVERFLOW =
      ((a >= 0 && b <  0 && r <  0) ||
       (a <  0 && b >= 0 && r >= 0));

  set_zero_flag(cpu, r);
}
/* --- core ---------------------------------------------------------------- */

// Fetch the next instruction from the given memory and cpu and increments the program counter
static void fetch(CPU* cpu, word* data_mem) {
  mem_addr next_instruction_addr = cpu->PC;
  cpu->IR = (instr)data_mem[next_instruction_addr];
  cpu->PC++;
}

// Loads the data at operand in data_mem into cpu's ACC register
static void load(CPU* cpu, word* data_mem, mem_addr operand) {
  cpu->ACC = (word)data_mem[operand];
  set_zero_flag(cpu, cpu->ACC);
}

// Stores the data in cpu's ACC register at the operand in data_mem
static void store(CPU* cpu, word* data_mem, mem_addr operand) {
  data_mem[operand] = cpu->ACC;
}

// Adds the value in the cpu's ACC register with the value at operand in data_mem
// The sum is stored in ACC and the appropiate flags are set
static void add(CPU* cpu, word* data_mem, mem_addr operand) {
  word a = cpu->ACC;
  word b = data_mem[operand];
  word r = (a + b);
  cpu->ACC = r;
  set_add_flags(cpu, a, b, r);
}

// Subtracts the value in the cpu's ACC register with the value at operand in data_mem
// The differnce is stored in ACC and the appropiate flags are set
static void sub(CPU* cpu, word* data_mem, mem_addr operand) {
  word a = cpu->ACC;
  word b = data_mem[operand];
  word r = (a - b);
  cpu->ACC = r;
  set_sub_flags(cpu, a, b, r);
}

// Halts execution of the given cpu
static void halt(CPU* cpu) {
  printf("HALT\n");
  cpu->PC = CPU_HALT;
}

// Executes the instruction in the given cpu's IR with the given RAM
static void execute(CPU* cpu, word* data_mem) {
  instr instruction = cpu->IR;
  Decoded d = decode(instruction);
  OP opcode = d.op;
  mem_addr operand = d.addr;

  switch (opcode) {
    case OP_LOAD:  load(cpu, data_mem, operand);  break;
    case OP_STORE: store(cpu, data_mem, operand); break;
    case OP_ADD:   add(cpu, data_mem, operand);   break;
    case OP_SUB:   sub(cpu, data_mem, operand);   break;
    case OP_HALT:  halt(cpu);                     break;
    default:
      printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode, (unsigned)instruction);
      cpu->PC = CPU_HALT;
  }
}

// Runs the fetch-execution cycle program_size times or until a halt is encountered
void cpu_run(CPU* cpu, int program_size, word* mem) {
  for (int i = 0; i < program_size && cpu->PC != CPU_HALT; i++) {
    printf("=== Cycle %d ===\n", i + 1);
    cpu_print_state(cpu);

    fetch(cpu, mem);
    execute(cpu, mem);

    if (cpu->PC == CPU_HALT) {
      printf("CPU Halted!\n");
      break;
    }
  }
}

// Prints the state of the given CPU
void cpu_print_state(const CPU* cpu) {
  printf("CPU STATE\n");
  printf("PC: %X\n", cpu->PC);
  printf("ACC: %X\n", cpu->ACC);
  printf("IR: %X\n", cpu->IR);
  printf("FLAGS:\n");
  printf("  ZERO: %1d\n", cpu->flags.ZERO);
  printf("  CARRY: %1d\n", cpu->flags.CARRY);
  printf("  OVERFLOW: %1d\n\n\n", cpu->flags.OVERFLOW);
}

int main() {
  struct flags flags = {0, 0, 0};
  struct cpu cpu = {0x300, 0, 0, flags}; 
  /*
  word memory[0x1000];
  memory[0x300] = 0x1940;
  memory[0x301] = 0x5941;
  memory[0x302] = 0x2941;
  memory[0x303] = 0x6942;
  memory[0x304] = 0x2942;
  memory[0x940] = 0x0003;
  memory[0x941] = 0x0002;
  memory[0x942] = 0x0001;
  */
  init_cache(&L1, L1CACHE_SIZE);
  init_cache(&L2, L2CACHE_SIZE);
  init_ram(RAM_SIZE);

  
  //cpu_run(&cpu, 5, memory);
  /*
  printf("0x941: %X\n", memory[0x941]);
  printf("0x942: %X\n", memory[0x942]);
  */

  write_mem(0x300, 0x1940);
  printf("read adress 0x300 <-> 0x%X\n", read_mem(0x300));
  cpu_run(&cpu, 5, RAM);
  print_cache_stats();
}

