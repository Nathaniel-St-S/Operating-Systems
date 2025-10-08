#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/isa.h"
#include "../include/memory.h"

// Loads the data at the given memory address into cpu's ACC register
void load(const mem_addr operand) {
  CPU.ACC = read_mem(operand);
  set_zero_flag(CPU.ACC);
}

// Stores the data in cpu's ACC register at the given memory address
void store(const mem_addr operand) {
  write_mem(operand, CPU.ACC);
}

// Adds the value in the cpu's ACC register with the given value
// The sum is stored in ACC and the appropriate flags are set
void add(const mem_addr operand) {
  word init = CPU.ACC;
  CPU.ACC += operand;
  set_add_flags(init, operand, CPU.ACC);
}

// Subtracts the value in the cpu's ACC register with the given value
// The difference is stored in ACC and the appropriate flags are set
void sub(const mem_addr operand) {
  word init = CPU.ACC;
  CPU.ACC -= operand;
  set_sub_flags(init, operand, CPU.ACC);
}

//Initiates and handles CPU interrupts
void interrupt(const mem_addr operand) { 
  set_interrupt_flag(operand);
}

// Halts execution of the given cpu
void halt() {
  printf("HALT\n");
  CPU.PC = CPU_HALT;
}

//execute the instruction based off of the opcode
void execute_instruction(const OP opcode, const mem_addr operand)
{
  switch (opcode) {
    case OP_LOAD:  load(operand);       break;
    case OP_STORE: store(operand);      break;
    case OP_ADD:   add(operand);        break;
    case OP_SUB:   sub(operand);        break;
    case OP_INTR:  interrupt(operand);  break;
    case OP_HALT:  halt();              break;
    default:
      printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode, (unsigned)operand);
      CPU.PC = CPU_HALT;
  }
}
