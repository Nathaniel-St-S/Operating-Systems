#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/isa.h"
#include "../include/memory.h"

// Loads the data at operand in data_mem into cpu's ACC register
static void load(CPU* cpu, mem_addr operand) {
  //cpu->ACC = (word)data_mem[operand];
  cpu->ACC = read_mem(operand);
  set_zero_flag(cpu, cpu->ACC);
}

// Stores the data in cpu's ACC register at the operand in data_mem
static void store(CPU* cpu, mem_addr operand) {
  //data_mem[operand] = cpu->ACC;
  write_mem(operand, cpu->ACC);
}

// Adds the value in the cpu's ACC register with the value at operand in data_mem
// The sum is stored in ACC and the appropiate flags are set
static void add(CPU* cpu, mem_addr operand) {
  word a = cpu->ACC;
  //word b = data_mem[operand];
  word b = read_mem(operand);
  word r = (a + b);
  cpu->ACC = r;
  set_add_flags(cpu, a, b, r);
}

// Subtracts the value in the cpu's ACC register with the value at operand in data_mem
// The differnce is stored in ACC and the appropiate flags are set
static void sub(CPU* cpu, mem_addr operand) {
  word a = cpu->ACC;
  //word b = data_mem[operand];
  word b = read_mem(operand);
  word r = (a - b);
  cpu->ACC = r;
  set_sub_flags(cpu, a, b, r);
}

// Halts execution of the given cpu
static void halt(CPU* cpu) {
  printf("HALT\n");
  cpu->PC = CPU_HALT;
}

//execute the instruction based off of the opcode
void execute_instruction(OP opcode, mem_addr operand)
{
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
