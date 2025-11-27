#ifndef ISA_H
#define ISA_H

#include <stdint.h>
#define CPU_HALT (uint32_t)0xFFFFFFFF
#define SET_FLAG(flag)   (THE_CPU.hw_registers[FLAGS] |= (flag))
#define CLEAR_FLAG(flag) (THE_CPU.hw_registers[FLAGS] &= ~(flag))
#define CLEAR_ALL_FLAGS  (THE_CPU.hw_registers[FLAGS] = 0)

// X_SHIFT is the number of bits to shift right
// X_MASK is the number to bitwise and the instruction with
#define OPCODE_SHIFT 26

#define RS_SHIFT 21
#define RS_MASK 0x1F

#define RT_SHIFT 16
#define RT_MASK 0x1F

#define RD_SHIFT 11
#define RD_MASK 0x1F

#define SHAMT_SHIFT 6
#define SHAMT_MASK 0x1F

#define FUNCT_MASK 0x3F

#define IMM_MASK 0xFFFFF
#define REG_MASK 0xFFFF

// Opcodes
enum {
  OP_ADD = 0x0,
  OP_ADDU = 0x0,
  OP_SUB = 0x0,
  OP_SUBU = 0x0,
  OP_MULT = 0x0,
  OP_MULTU = 0x0,
  OP_DIV = 0x0,
  OP_DIVU = 0x0,
  OP_MFHI = 0x0,
  OP_MFLO = 0x0,
  OP_MTHI = 0x0,
  OP_MTLO = 0x0,
  OP_AND = 0x0,
  OP_OR = 0x0,
  OP_XOR = 0x0,
  OP_NOR = 0x0,
  OP_SLL = 0x0,
  OP_SRL = 0x0,
  OP_SRA = 0x0,
  OP_SLLV = 0x0,
  OP_SRLV = 0x0,
  OP_SRAV = 0x0,
  OP_ADDI = 0x08,
  OP_ADDIU = 0x09,
  OP_ANDI = 0x0C,
  OP_ORI = 0x0D,
  OP_XORI = 0x0E,
  OP_SLTI = 0x0A,
  OP_SLTIU = 0x0B,
  OP_LUI = 0x0F,
  OP_LW = 0x23,
  OP_SW = 0x2B,
  OP_LB = 0x20,
  OP_LBU = 0x24,
  OP_LH = 0x21,
  OP_LHU = 0x25,
  OP_SB = 0x28,
  OP_SH = 0x29,
  OP_BEQ = 0x04,
  OP_BNE = 0x05,
  OP_J = 0x02,
  OP_JAL = 0x03,
  OP_JR = 0x0,
  OP_JALR = 0x0,
  OP_SYSCALL = 0x0,
  OP_BREAK = 0x0,
  OP_ERET = 0x10,
};

enum {
  FUNCT_ADD = 0x20,
  FUNCT_ADDU = 0x21,
  FUNCT_SUB = 0x22,
  FUNCT_SUBU = 0x23,
  FUNCT_MULT = 0x18,
  FUNCT_MULTU = 0x19,
  FUNCT_DIV = 0x1A,
  FUNCT_DIVU = 0x1B,
  FUNCT_MFHI = 0x10,
  FUNCT_MFLO = 0x12,
  FUNCT_MTHI = 0x11,
  FUNCT_MTLO = 0x13,
  FUNCT_AND = 0x24,
  FUNCT_OR = 0x25,
  FUNCT_XOR = 0x26,
  FUNCT_NOR = 0x27,
  FUNCT_SLL = 0x00,
  FUNCT_SRL = 0x02,
  FUNCT_SRA = 0x03,
  FUNCT_SLLV = 0x04,
  FUNCT_SRLV = 0x06,
  FUNCT_SRAV = 0x07,
  FUNCT_JR = 0x08,
  FUNCT_JALR = 0x09,
  FUNCT_SYSCALL = 0x0C,
  FUNCT_BREAK = 0x0D,
};

void execute_instruction(uint32_t instruction);

#endif // !ISA_H
