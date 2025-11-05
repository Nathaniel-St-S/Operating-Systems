#ifndef ISA_H
#define ISA_H
#include "types.h"

#define CPU_HALT (dword)0xFFFFFFFF
#define SET_FLAG(flag)   (THE_CPU.registers[FLAG] |= (flag))
#define CLEAR_FLAG(flag) (THE_CPU.registers[FLAG] &= ~(flag))
#define CLEAR_ALL_FLAGS  (THE_CPU.registers[FLAG] = 0)

#define OPCODE_SHIFT 24
#define OPCODE_MASK  0xFF000000

#define DR_SHIFT     20
#define DR_MASK      0x00F00000

#define SR1_SHIFT    16
#define SR1_MASK     0x000F0000

#define MODE_SHIFT   12
#define MODE_MASK    0x0000F000

#define OPERAND_MASK 0x00000FFF

#define IMM_MASK 0xFFFFF
#define REG_MASK 0xFFFF


// Opcodes
enum
{
  ADD = 0x0,
  SUB,
  MUL,
  DIV,
  AND,
  OR,
  XOR,
  NOT,
  BRANCH,
  JUMP,
  JUMPR,
  JUMPZ,
  STORE,
  STRR,
  STRI,
  LOAD,
  LEA,
  LDR,
  LDI,
  INTR,
};

// execute instruction based off of opcode
void execute_instruction(dword op, dword isntruction);

#endif
