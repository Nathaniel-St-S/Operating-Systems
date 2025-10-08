#ifndef ISA_H
#define ISA_H
#include "types.h"

#define CPU_HALT (word)0xFFFF

/**
 * The supported operations of the CPU
 */
typedef enum {
  LOAD = 0x1,
  STORE = 0x2,
  ADD = 0x3,    // ACC + x
  SUB = 0x4,    // ACC - x
  MUL = 0x5,    // ACC * x
  DIV = 0x6,    // ACC / x
  AND = 0x7,    // ACC & x
  OR = 0x8,     // ACC | x
  JMP = 0x9,    // Jump to a specified memory address
  JZ = 0xA,     // My favorite rapper
  INTR = 0xB,   // Interrupt
  ENDINT = 0xC, // End Interrupt
  HALT = 0xF
} OP;

// execute instruction based off of opcode
void execute_instruction(OP op, mem_addr addr);

#endif
