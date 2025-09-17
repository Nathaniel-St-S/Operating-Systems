#ifndef ISA_H
#define ISA_H

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

//execute instruction based off of opcode
static void execute_instruction(OP op, mem_addr addr);

// Loads the data at operand in data_mem into cpu's ACC register
static void load(mem_addr operand);

// Stores the data in cpu's ACC register at the operand in data_mem
static void store(mem_addr operand);

// Adds the value in the cpu's ACC register with the value at operand in data_mem
// The sum is stored in ACC and the appropiate flags are set
static void add(mem_addr operand);

// Subtracts the value in the cpu's ACC register with the value at operand in data_mem
// The differnce is stored in ACC and the appropiate flags are set
static void sub(mem_addr operand);

// Halts execution of the given cpu
static void halt(void);

#endif
