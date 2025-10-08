#include "../include/isa.h"
#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/types.h"

static inline void load(const mem_addr operand);
static inline void store(const mem_addr operand);
static inline void add(const mem_addr operand);
static inline void sub(const mem_addr operand);
static inline void mul(const mem_addr operand);
static inline void divide(const mem_addr operand);
static inline void bit_and(const mem_addr operand);
static inline void bit_or(const mem_addr operand);
static inline void jump(const mem_addr operand);
static inline void jz(const mem_addr operand);
static inline void interrupt(const mem_addr operand);
static inline void halt();
static inline void report_invalid_opcode(const OP opcode, const mem_addr operand);
void execute_instruction(const OP opcode, const mem_addr operand);

// Loads the data at the given memory address into cpu's ACC register
static inline void load(const mem_addr operand) {
  CPU.ACC = read_mem(operand);
  set_zero_flag(CPU.ACC);
}

// Stores the data in cpu's ACC register at the given memory address
static inline void store(const mem_addr operand) { write_mem(operand, CPU.ACC); }

// Adds the value in the cpu's ACC register with the given value
// The sum is stored in ACC and the appropriate flags are set
static inline void add(const mem_addr operand) {
  word val = read_mem(operand);
  word init = CPU.ACC;
  CPU.ACC += val;
  set_add_flags(init, val, CPU.ACC);
}

// Subtracts the value in the cpu's ACC register with the given value
// The difference is stored in ACC and the appropriate flags are set
static inline void sub(const mem_addr operand) {
  word val = read_mem(operand);
  word init = CPU.ACC;
  CPU.ACC -= val;
  set_sub_flags(init, val, CPU.ACC);
}

// Multiplies the value in the cpu's ACC register with the value at the given
// memory address. The product is stored in ACC and the appropriate flags are
// set.
static inline void mul(const mem_addr operand) {
  word val = read_mem(operand);
  word init = CPU.ACC;
  CPU.ACC *= val;
  // TODO: set_mul_flags(init, val, CPU.ACC)
}

// Divides the value in the cpu's ACC register with the value at the given
// memory address. The quotient is stored in the ACC and the appropriate flags
// are set.
static inline void divide(const mem_addr operand) {
  word val = read_mem(operand);
  if (!val) {
    printf("ERROR: DIVIDE BY ZERO");
    CPU.PC = CPU_HALT;
    return;
  }
  word init = CPU.ACC;
  CPU.ACC /= val;
  // TODO: set_div_flags(init, val, CPU.ACC)
}

// Performs bitwise and on the cpu's ACC rgister and the value at the given
// memory address.
static inline void bit_and(const mem_addr operand) {
  word val = read_mem(operand);
  CPU.ACC &= val;
}

// Performs bitwise or on the cpu's ACC rgister and the value at the given
// memory address.
static inline void bit_or(const mem_addr operand) {
  word val = read_mem(operand);
  CPU.ACC |= val;
}

// Begins execution at the given memory address by setting the cpu's program
// counter.
static inline void jump(const mem_addr operand) { CPU.PC = operand; }

// Begins execution at the given memory address if the zero flag is set.
static inline void jump_zero(const mem_addr operand) {
  if (CPU.flags.ZERO) {
    CPU.PC = operand;
  }
}

// Initiates and handles CPU interrupts
static inline void interrupt(const mem_addr operand) { set_interrupt_flag(operand); }

// Halts execution of the given cpu
static inline void halt() {
  printf("HALT\n");
  CPU.PC = CPU_HALT;
}

// Reports an error for invalid opcodes and halts the CPU.
static inline void report_invalid_opcode(const OP opcode, const mem_addr operand) {
  printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode,
          (unsigned)operand);
  CPU.PC = CPU_HALT;
}

// execute the instruction based off of the opcode
void execute_instruction(const OP opcode, const mem_addr operand) {
  switch (opcode) {
    case LOAD: load(operand); break;
    case STORE: store(operand); break;
    case ADD: add(operand); break;
    case SUB: sub(operand); break;
    case MUL: mul(operand); break;
    case DIV: divide(operand); break;
    case AND: bit_and(operand); break;
    case OR: bit_or(operand); break;
    case JMP: jump(operand); break;
    case JZ: jump(operand); break;
    case INTR: interrupt(operand); break;
    case HALT: halt(); break;
    default: report_invalid_opcode(opcode, operand);
  }
}
