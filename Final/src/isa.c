#include "../include/isa.h"
#include "../include/cpu.h"



static inline uword mask_reg_index(word reg) {
  return (uword)reg & 0x1F;
}

static inline word read_gpr(word reg) {
  uword idx = mask_reg_index(reg);
  if (idx == REG_ZERO || idx >= GP_REG_COUNT) {
    return 0;
  }
  return THE_CPU.gp_registers[idx];
}

static inline void write_gpr(word reg, word value) {
  uword idx = mask_reg_index(reg);
  if (idx == REG_ZERO || idx >= GP_REG_COUNT) {
    return;
  }
  THE_CPU.gp_registers[idx] = value;
}

static inline uword mask_shift_amount(word value) {
  return (uword)value & 0x1F;
}

static void add(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  word result = read_gpr(rs) + read_gpr(rt);
  write_gpr(rd, result);
}

static void addu(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  uword lhs = (uword)read_gpr(rs);
  uword rhs = (uword)read_gpr(rt);
  write_gpr(rd, (word)(lhs + rhs));
}

static void sub(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  word result = read_gpr(rs) - read_gpr(rt);
  write_gpr(rd, result);
}

static void subu(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  uword lhs = (uword)read_gpr(rs);
  uword rhs = (uword)read_gpr(rt);
  write_gpr(rd, (word)(lhs - rhs));
}

static void mult(word rs, word rt, word rd, word shamt) {
  (void)rd;
  (void)shamt;
  int64_t product = (int64_t)read_gpr(rs) * (int64_t)read_gpr(rt);
  THE_CPU.hw_registers[HI] = (word)(product >> 32);
  THE_CPU.hw_registers[LO] = (word)product;
}

static void multu(word rs, word rt, word rd, word shamt) {
  (void)rd;
  (void)shamt;
  uint64_t product = (uint64_t)(uword)read_gpr(rs) * (uint64_t)(uword)read_gpr(rt);
  THE_CPU.hw_registers[HI] = (word)(product >> 32);
  THE_CPU.hw_registers[LO] = (word)product;
}

static void div(word rs, word rt, word rd, word shamt) {
  (void)rd;
  (void)shamt;
  word divisor = read_gpr(rt);
  if (divisor == 0) {
    return;
  }
  word dividend = read_gpr(rs);
  THE_CPU.hw_registers[LO] = dividend / divisor;
  THE_CPU.hw_registers[HI] = dividend % divisor;
}

static void divu(word rs, word rt, word rd, word shamt) {
  (void)rd;
  (void)shamt;
  uword divisor = (uword)read_gpr(rt);
  if (divisor == 0) {
    return;
  }
  uword dividend = (uword)read_gpr(rs);
  THE_CPU.hw_registers[LO] = (word)(dividend / divisor);
  THE_CPU.hw_registers[HI] = (word)(dividend % divisor);
}

static void mfhi(word rs, word rt, word rd, word shamt) {
  (void)rs;
  (void)rt;
  (void)shamt;
  write_gpr(rd, THE_CPU.hw_registers[HI]);
}

static void mflo(word rs, word rt, word rd, word shamt) {
  (void)rs;
  (void)rt;
  (void)shamt;
  write_gpr(rd, THE_CPU.hw_registers[LO]);
}

static void mthi(word rs, word rt, word rd, word shamt) {
  (void)rt;
  (void)rd;
  (void)shamt;
  THE_CPU.hw_registers[HI] = read_gpr(rs);
}

static void mtlo(word rs, word rt, word rd, word shamt) {
  (void)rt;
  (void)rd;
  (void)shamt;
  THE_CPU.hw_registers[LO] = read_gpr(rs);
}

static void and(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  write_gpr(rd, read_gpr(rs) & read_gpr(rt));
}

static void or(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  write_gpr(rd, read_gpr(rs) | read_gpr(rt));
}

static void xor(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  write_gpr(rd, read_gpr(rs) ^ read_gpr(rt));
}

static void nor(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  write_gpr(rd, ~(read_gpr(rs) | read_gpr(rt)));
}

static void sll(word rs, word rt, word rd, word shamt) {
  (void)rs;
  uword amount = mask_shift_amount(shamt);
  uword value = (uword)read_gpr(rt);
  write_gpr(rd, (word)(value << amount));
}

static void srl(word rs, word rt, word rd, word shamt) {
  (void)rs;
  uword amount = mask_shift_amount(shamt);
  uword value = (uword)read_gpr(rt);
  write_gpr(rd, (word)(value >> amount));
}

static void sra(word rs, word rt, word rd, word shamt) {
  (void)rs;
  uword amount = mask_shift_amount(shamt);
  word value = read_gpr(rt);
  write_gpr(rd, value >> amount);
}

static void sllv(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  uword amount = mask_shift_amount(read_gpr(rs));
  uword value = (uword)read_gpr(rt);
  write_gpr(rd, (word)(value << amount));
}

static void srlv(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  uword amount = mask_shift_amount(read_gpr(rs));
  uword value = (uword)read_gpr(rt);
  write_gpr(rd, (word)(value >> amount));
}

static void srav(word rs, word rt, word rd, word shamt) {
  (void)shamt;
  uword amount = mask_shift_amount(read_gpr(rs));
  word value = read_gpr(rt);
  write_gpr(rd, value >> amount);
}

static void addi(uword instruction) {

}

static void addiu(uword instruction) {

}

static void andi(uword instruction) {

}

static void ori(uword instruction) {

}

static void xori(uword instruction) {

}

static void slti(uword instruction) {

}

static void sltiu(uword instruction) {

}

static void lui(uword instruction) {

}

static void lw(uword instruction) {

}

static void sw(uword instruction) {

}

static void lb(uword instruction) {

}

static void lbu(uword instruction) {

}

static void lh(uword instruction) {

}

static void lhu(uword instruction) {

}

static void sb(uword instruction) {

}

static void sh(uword instruction) {

}

static void beq(uword instruction) {

}

static void bne(uword instruction) {

}

static void j(uword instruction) {

}

static void jal(uword instruction) {

}

static void jr(word rs, word rt, word rd, word shamt) {
  (void)rt;
  (void)rd;
  (void)shamt;
  THE_CPU.hw_registers[PC] = read_gpr(rs);
}

static void jalr(word rs, word rt, word rd, word shamt) {
  (void)rt;
  (void)shamt;
  word target = read_gpr(rs);
  word next_pc = THE_CPU.hw_registers[PC];
  write_gpr(rd, next_pc);
  THE_CPU.hw_registers[PC] = target;
}

static void syscall(word rs, word rt, word rd, word shamt) {
  (void)rs;
  (void)rt;
  (void)rd;
  (void)shamt;
  THE_CPU.hw_registers[PC] = CPU_HALT;
}

static void breakk(word rs, word rt, word rd, word shamt) {
  (void)rs;
  (void)rt;
  (void)rd;
  (void)shamt;
  THE_CPU.hw_registers[PC] = CPU_HALT;
}

static void eret(uword instruction) {

}

static void handle_r_type_instruction(uword instruction) {
  Funct funct = (Funct)(instruction & 0x3F);
  word rs = (instruction >> 21) & 0x1F;
  word rt = (instruction >> 16) & 0x1F;
  word rd = (instruction >> 11) & 0x1F;
  word shamt = (instruction >> 6) & 0x1F;
  switch (funct) {
    case FUNCT_ADD: add(rs, rt, rd, shamt); break;
    case FUNCT_ADDU: addu(rs, rt, rd, shamt); break;
    case FUNCT_SUB: sub(rs, rt, rd, shamt); break;
    case FUNCT_SUBU: subu(rs, rt, rd, shamt); break;
    case FUNCT_MULT: mult(rs, rt, rd, shamt); break;
    case FUNCT_MULTU: multu(rs, rt, rd, shamt); break;
    case FUNCT_DIV: div(rs, rt, rd, shamt); break;
    case FUNCT_DIVU: divu(rs, rt, rd, shamt); break;
    case FUNCT_MFHI: mfhi(rs, rt, rd, shamt); break;
    case FUNCT_MFLO: mflo(rs, rt, rd, shamt); break;
    case FUNCT_MTHI: mthi(rs, rt, rd, shamt); break;
    case FUNCT_MTLO: mtlo(rs, rt, rd, shamt); break;
    case FUNCT_AND: and(rs, rt, rd, shamt); break;
    case FUNCT_OR: or(rs, rt, rd, shamt); break;
    case FUNCT_XOR: xor(rs, rt, rd, shamt); break;
    case FUNCT_NOR: nor(rs, rt, rd, shamt); break;
    case FUNCT_SLL: sll(rs, rt, rd, shamt); break;
    case FUNCT_SRL: srl(rs, rt, rd, shamt); break;
    case FUNCT_SRA: sra(rs, rt, rd, shamt); break;
    case FUNCT_SLLV: sllv(rs, rt, rd, shamt); break;
    case FUNCT_SRLV: srlv(rs, rt, rd, shamt); break;
    case FUNCT_SRAV: srav(rs, rt, rd, shamt); break;
    case FUNCT_JR: jr(rs, rt, rd, shamt); break;
    case FUNCT_JALR: jalr(rs, rt, rd, shamt); break;
    case FUNCT_SYSCALL: syscall(rs, rt, rd, shamt); break;
    case FUNCT_BREAK: breakk(rs, rt, rd, shamt); break;
  }
}


void execute_instruction(uword instruction) {
  Opcode opcode = instruction >> 26;
  switch (opcode) {
    // R type instructions
    case 0x0: handle_r_type_instruction(instruction); break;
    // I type instructions
    case OP_ADDI: addi(instruction); break;
    case OP_ADDIU: addiu(instruction); break;
    case OP_ANDI: andi(instruction); break;
    case OP_ORI: ori(instruction); break;
    case OP_XORI: xori(instruction); break;
    case OP_SLTI: slti(instruction); break;
    case OP_SLTIU: sltiu(instruction); break;
    case OP_LUI: lui(instruction); break;
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
