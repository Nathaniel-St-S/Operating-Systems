#include "../include/isa.h"
#include "../include/cpu.h"
#include <stdint.h>



static inline uint32_t mask_reg_index(uint32_t reg) {
  return (uint32_t)reg & 0x1F;
}

inline int32_t read_gpr(uint32_t reg) {
  uint32_t idx = mask_reg_index(reg);
  if (idx == REG_ZERO || idx >= GP_REG_COUNT) {
    return 0;
  }
  return THE_CPU.gp_registers[idx];
}

inline void write_gpr(uint32_t reg, uint32_t value) {
  uint32_t idx = mask_reg_index(reg);
  if (idx == REG_ZERO || idx >= GP_REG_COUNT) {
    return;
  }
  THE_CPU.gp_registers[idx] = value;
}

static inline uint32_t mask_shift_amount(uint32_t value) {
  return value & 0x1F;
}

static inline uint32_t get_opcode(uint32_t instruction) {
  return instruction >> OPCODE_SHIFT;
}

// ---------------R Type Instructions---------------------
static void add(uint32_t rs, uint32_t rt, uint32_t rd) {
  int32_t lhs = read_gpr(rs);
  int32_t rhs = read_gpr(rt);
  write_gpr(rd, lhs + rhs);
}

static void addu(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  write_gpr(rd, (uint32_t)(lhs + rhs));
}

static void sub(uint32_t rs, uint32_t rt, uint32_t rd) {
  int32_t lhs = read_gpr(rs);
  int32_t rhs = read_gpr(rt);
  write_gpr(rd, lhs - rhs);
}

static void subu(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  write_gpr(rd, (lhs - rhs));
}

static void mult(uint32_t rs, uint32_t rt) {
  int32_t lhs = read_gpr(rs);
  int32_t rhs = read_gpr(rt);
  int64_t product = (int64_t)lhs * (int64_t)rhs;
  THE_CPU.hw_registers[HI] = (uint32_t)(product >> 32);
  THE_CPU.hw_registers[LO] = (uint32_t)product;
}

static void multu(uint32_t rs, uint32_t rt) {
  uint64_t lhs = (uint64_t)(uint32_t)read_gpr(rs);
  uint64_t rhs = (uint64_t)(uint32_t)read_gpr(rt);
  uint64_t product = lhs * rhs;
  THE_CPU.hw_registers[HI] = (uint32_t)(product >> 32);
  THE_CPU.hw_registers[LO] = (uint32_t)product;
}

static void div(uint32_t rs, uint32_t rt) {
  int32_t divisor = read_gpr(rt);
  if (divisor == 0) {
    return; // TODO: Throw an exception here
  }
  int32_t dividend = read_gpr(rs);
  THE_CPU.hw_registers[LO] = (uint32_t)(dividend / divisor);
  THE_CPU.hw_registers[HI] = (uint32_t)(dividend % divisor);
}

static void divu(uint32_t rs, uint32_t rt) {
  uint32_t divisor = (uint32_t)read_gpr(rt);
  if (divisor == 0) {
    return; // TODO: Throw an exception here
  }
  uint32_t dividend = (uint32_t)read_gpr(rs);
  THE_CPU.hw_registers[LO] = (dividend / divisor);
  THE_CPU.hw_registers[HI] = (dividend % divisor);
}

static void mfhi(uint32_t rd) {
  write_gpr(rd, THE_CPU.hw_registers[HI]);
}

static void mflo(uint32_t rd) {
  write_gpr(rd, THE_CPU.hw_registers[LO]);
}

static void mthi(uint32_t rs) {
  THE_CPU.hw_registers[HI] = read_gpr(rs);
}

static void mtlo(uint32_t rs) {
  THE_CPU.hw_registers[LO] = read_gpr(rs);
}

static void and(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  write_gpr(rd, lhs & rhs);
}

static void or(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  write_gpr(rd, lhs | rhs);
}

static void xor(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  write_gpr(rd, lhs ^ rhs);
}

static void nor(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  write_gpr(rd, ~(lhs | rhs));
}

static void sll(uint32_t rt, uint32_t rd, uint32_t shamt) {
  uint32_t amount = mask_shift_amount(shamt);
  uint32_t value = (uint32_t)read_gpr(rt);
  write_gpr(rd, (uint32_t)(value << amount));
}

static void srl(uint32_t rt, uint32_t rd, uint32_t shamt) {
  uint32_t amount = mask_shift_amount(shamt);
  uint32_t value = (uint32_t)read_gpr(rt);
  write_gpr(rd, value >> amount);
}

static void sra(uint32_t rt, uint32_t rd, uint32_t shamt) {
  uint32_t amount = mask_shift_amount(shamt);
  int32_t value = (int32_t)read_gpr(rt);
  write_gpr(rd, (uint32_t)(value >> amount));
}

static void sllv(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t amount = mask_shift_amount((uint32_t)read_gpr(rs));
  uint32_t value = (uint32_t)read_gpr(rt);
  write_gpr(rd, value << amount);
}

static void srlv(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t amount = mask_shift_amount((uint32_t)read_gpr(rs));
  uint32_t value = (uint32_t)read_gpr(rt);
  write_gpr(rd, value >> amount);
}

static void srav(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t amount = mask_shift_amount((uint32_t)read_gpr(rs));
  int32_t value = read_gpr(rt);
  write_gpr(rd, (uint32_t)(value >> amount));
}

static void jr(uint32_t rs) {
  THE_CPU.hw_registers[PC] = read_gpr(rs);
}

static void jalr(uint32_t rs, uint32_t rd) {
  uint32_t target = read_gpr(rs);
  uint32_t next_pc = THE_CPU.hw_registers[PC];
  write_gpr(rd, next_pc);
  THE_CPU.hw_registers[PC] = target;
}

static void syscall() {
  THE_CPU.hw_registers[PC] = CPU_HALT;
}

static void breakk() {
  THE_CPU.hw_registers[PC] = CPU_HALT;
}

static void handle_r_type_instruction(uint32_t instruction) {
  uint32_t funct = instruction & FUNCT_MASK;
  uint32_t rs = (instruction >> RS_SHIFT) & RS_MASK;
  uint32_t rt = (instruction >> RT_SHIFT) & RT_MASK;
  uint32_t rd = (instruction >> RD_SHIFT) & RD_MASK;
  uint32_t shamt = (instruction >> SHAMT_SHIFT) & SHAMT_MASK;
  switch (funct) {
    case FUNCT_ADD: add(rs, rt, rd); break;
    case FUNCT_ADDU: addu(rs, rt, rd); break;
    case FUNCT_SUB: sub(rs, rt, rd); break;
    case FUNCT_SUBU: subu(rs, rt, rd); break;
    case FUNCT_MULT: mult(rs, rt); break;
    case FUNCT_MULTU: multu(rs, rt); break;
    case FUNCT_DIV: div(rs, rt); break;
    case FUNCT_DIVU: divu(rs, rt); break;
    case FUNCT_MFHI: mfhi(rd); break;
    case FUNCT_MFLO: mflo(rd); break;
    case FUNCT_MTHI: mthi(rs); break;
    case FUNCT_MTLO: mtlo(rs); break;
    case FUNCT_AND: and(rs, rt, rd); break;
    case FUNCT_OR: or(rs, rt, rd); break;
    case FUNCT_XOR: xor(rs, rt, rd); break;
    case FUNCT_NOR: nor(rs, rt, rd); break;
    case FUNCT_SLL: sll(rt, rd, shamt); break;
    case FUNCT_SRL: srl(rt, rd, shamt); break;
    case FUNCT_SRA: sra(rt, rd, shamt); break;
    case FUNCT_SLLV: sllv(rs, rt, rd); break;
    case FUNCT_SRLV: srlv(rs, rt, rd); break;
    case FUNCT_SRAV: srav(rs, rt, rd); break;
    case FUNCT_JR: jr(rs); break;
    case FUNCT_JALR: jalr(rs, rd); break;
    case FUNCT_SYSCALL: syscall(); break;
    case FUNCT_BREAK: breakk(); break;
  }
}

// ---------------I Type Instructions---------------------

static void addi(uint32_t rs, uint32_t rt, uint16_t imm) {
  int32_t simm = (int16_t)imm;
  int32_t lhs = read_gpr(rs);
  write_gpr(rt, (uint32_t)(lhs + simm));
}

static void addiu(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t simm = (uint32_t)(int32_t)(int16_t)imm;
  uint32_t lhs = (uint32_t)read_gpr(rs);
  write_gpr(rt, lhs + simm);
}

static void andi(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  write_gpr(rt, lhs & imm);
}

static void ori(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  write_gpr(rt, lhs | imm);
}

static void xori(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  write_gpr(rt, lhs ^ imm);
}

static void slti(uint32_t rs, uint32_t rt, uint16_t imm) {
  int32_t simm = (int16_t)imm;
  int32_t lhs = read_gpr(rs);
  write_gpr(rt, lhs < simm ? 1 : 0);
}

static void sltiu(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t simm = (uint32_t)(int32_t)(int16_t)imm;
  uint32_t lhs = (uint32_t)read_gpr(rs);
  write_gpr(rt, lhs < simm ? 1 : 0);
}

static void lui(uint32_t rt, uint16_t imm) {
  write_gpr(rt, ((uint32_t)imm) << 16);
}

static int handle_immediate_instruction(uint32_t instruction) {
  uint32_t opcode = get_opcode(instruction);
  uint32_t rs = (instruction >> RS_SHIFT) & RS_MASK;
  uint32_t rt = (instruction >> RT_SHIFT) & RT_MASK;
  uint16_t imm = instruction & 0xFFFF;
  switch (opcode) {
    case OP_ADDI: addi(rs, rt, imm); return 1;
    case OP_ADDIU: addiu(rs, rt, imm); return 1;
    case OP_ANDI: andi(rs, rt, imm); return 1;
    case OP_ORI: ori(rs, rt, imm); return 1;
    case OP_XORI: xori(rs, rt, imm); return 1;
    case OP_SLTI: slti(rs, rt, imm); return 1;
    case OP_SLTIU: sltiu(rs, rt, imm); return 1;
    case OP_LUI: lui(rt, imm); return 1;
    default: return 0;
  }
}

static void lw(uint32_t instruction) {

}

static void sw(uint32_t instruction) {

}

static void lb(uint32_t instruction) {

}

static void lbu(uint32_t instruction) {

}

static void lh(uint32_t instruction) {

}

static void lhu(uint32_t instruction) {

}

static void sb(uint32_t instruction) {

}

static void sh(uint32_t instruction) {

}

static void beq(uint32_t instruction) {

}

static void bne(uint32_t instruction) {

}

static void j(uint32_t instruction) {

}

static void jal(uint32_t instruction) {

}

static void eret(uint32_t instruction) {

}

void execute_instruction(uint32_t instruction) {
  uint32_t opcode = get_opcode(instruction);
  // R type instructions all have an opcode of 0
  if (opcode == 0x0) {
    handle_r_type_instruction(instruction);
    return;
  }
  // I type instructions
  if (handle_immediate_instruction(instruction)) {
    return;
  }

  switch (opcode) {
    case OP_LW: lw(instruction); break;
    case OP_SW: sw(instruction); break;
    case OP_LB: lb(instruction); break;
    case OP_LBU: lbu(instruction); break;
    case OP_LH: lh(instruction); break;
    case OP_LHU: lhu(instruction); break;
    case OP_SB: sb(instruction); break;
    case OP_SH: sh(instruction); break;
    case OP_BEQ: beq(instruction); break;
    case OP_BNE: bne(instruction); break;
    // J type instructions
    case OP_J: j(instruction); break;
    case OP_JAL: jal(instruction); break;
    // Special Case
    case OP_ERET: eret(instruction); break;
  }
}
