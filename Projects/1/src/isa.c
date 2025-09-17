#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/isa.h"
#include "../include/memory.h"

// Loads the data at operand in data_mem into cpu's ACC register
void load(mem_addr operand) {
  CPU.ACC = read_mem(operand);
  set_zero_flag(CPU.ACC);
}

// Stores the data in cpu's ACC register at the operand in data_mem
void store(mem_addr operand) {
  write_mem(operand, CPU.ACC);
}

// Adds the value in the cpu's ACC register with the value at operand in data_mem
// The sum is stored in ACC and the appropiate flags are set
void add(mem_addr operand) {
  word a = CPU.ACC;
  word b = read_mem(operand);
  word r = (a + b);
  CPU.ACC = r;
  set_add_flags(a, b, r);
}

// Subtracts the value in the cpu's ACC register with the value at operand in data_mem
// The differnce is stored in ACC and the appropiate flags are set
void sub(mem_addr operand) {
  word a = CPU.ACC;
  word b = read_mem(operand);
  word r = (a - b);
  CPU.ACC = r;
  set_sub_flags(a, b, r);
}

// Halts execution of the given cpu
void halt() {
  printf("HALT\n");
  CPU.PC = CPU_HALT;
}

//execute the instruction based off of the opcode
void execute_instruction(OP opcode, mem_addr operand)
{
  switch (opcode) {
    case OP_LOAD:  load(operand);  break;
    case OP_STORE: store(operand); break;
    case OP_ADD:   add(operand);   break;
    case OP_SUB:   sub(operand);   break;
    case OP_HALT:  halt();         break;
    default:
      printf("ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode, (unsigned)operand);
      CPU.PC = CPU_HALT;
  }
}
