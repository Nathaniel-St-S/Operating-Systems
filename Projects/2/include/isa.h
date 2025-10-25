#ifndef ISA_H
#define ISA_H
#include "types.h"

#define CPU_HALT (word)0xFFFF
#define SET_FLAG(flag)   (THE_CPU.registers[FLAG] |= (flag))
#define CLEAR_FLAG(flag) (THE_CPU.registers[FLAG] &= ~(flag))
#define CLEAR_ALL_FLAGS  (THE_CPU.registers[FLAG] = 0)

// Opcodes
enum
{
  ADD = 0x0,
  SUB,
  MUL,
  DIV,
  AND,
  OR,
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
void execute_instruction(word op, word isntruction);

#endif
