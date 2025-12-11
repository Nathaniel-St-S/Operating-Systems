#include "../include/isa.h"
#include "../include/cpu.h"
#include "../include/memory.h"
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/time.h>


static inline uint32_t mask_reg_index(uint32_t reg) {
  return (uint32_t)reg & 0x1F;
}

static inline int32_t read_gpr(uint32_t reg) {
  uint32_t idx = mask_reg_index(reg);
  if (idx == REG_ZERO || idx >= GP_REG_COUNT) {
    return 0;
  }
  return THE_CPU.gp_registers[idx];
}

static inline void write_gpr(uint32_t reg, uint32_t value) {
  uint32_t idx = mask_reg_index(reg);
  if (idx == REG_ZERO || idx >= GP_REG_COUNT) {
    return;
  }
  THE_CPU.gp_registers[idx] = value;
}

static inline uint32_t mask_shift_amount(uint32_t value) {
  return value & 0x1F;
}

static inline int32_t sign_extend(uint32_t value, uint32_t bits) {
  if (bits == 0) {
    return 0;
  }
  if (bits >= 32) {
    return (int32_t)value;
  }
  uint32_t shift = 32 - bits;
  return (int32_t)((int32_t)(value << shift) >> shift);
}

static inline uint32_t zero_extend(uint32_t value, uint32_t bits) {
  if (bits >= 32) {
    return value;
  }
  uint32_t mask = (1u << bits) - 1u;
  return value & mask;
}

static void update_zero_flag(uint32_t value) {
  if (value == 0) {
    SET_FLAG(F_ZERO);
  } else {
    CLEAR_FLAG(F_ZERO);
  }
}

static void set_add_flags(uint32_t lhs, uint32_t rhs, uint32_t result) {
  CLEAR_FLAG(F_CARRY);
  CLEAR_FLAG(F_OVERFLOW);

  uint64_t usum = (uint64_t)lhs + (uint64_t)rhs;
  if (usum > UINT32_MAX) {
    SET_FLAG(F_CARRY);
  }

  int64_t ssum = (int64_t)(int32_t)lhs + (int64_t)(int32_t)rhs;
  if (ssum > INT32_MAX || ssum < INT32_MIN) {
    SET_FLAG(F_OVERFLOW);
  }

  update_zero_flag(result);
}

static void set_sub_flags(uint32_t lhs, uint32_t rhs, uint32_t result) {
  CLEAR_FLAG(F_CARRY);
  CLEAR_FLAG(F_OVERFLOW);

  if (lhs < rhs) {
    SET_FLAG(F_CARRY);
  }

  int64_t sdiff = (int64_t)(int32_t)lhs - (int64_t)(int32_t)rhs;
  if (sdiff > INT32_MAX || sdiff < INT32_MIN) {
    SET_FLAG(F_OVERFLOW);
  }

  update_zero_flag(result);
}

static void set_mul_flags(uint32_t lhs, uint32_t rhs, uint32_t lo,
                          uint32_t hi, bool is_signed) {
  CLEAR_FLAG(F_CARRY);
  CLEAR_FLAG(F_OVERFLOW);

  if (hi != 0) {
    SET_FLAG(F_CARRY);
  }

  if (is_signed) {
    int64_t product = (int64_t)(int32_t)lhs * (int64_t)(int32_t)rhs;
    if (product > INT32_MAX || product < INT32_MIN) {
      SET_FLAG(F_OVERFLOW);
    }
  }

  update_zero_flag(lo);
}

static void set_div_flags(uint32_t lhs, uint32_t rhs, uint32_t quotient,
                          bool is_signed) {
  CLEAR_FLAG(F_CARRY);
  CLEAR_FLAG(F_OVERFLOW);

  if (is_signed) {
    int32_t s_lhs = (int32_t)lhs;
    int32_t s_rhs = (int32_t)rhs;
    if (s_lhs == INT32_MIN && s_rhs == -1) {
      SET_FLAG(F_OVERFLOW);
    }
  }

  update_zero_flag(quotient);
}

static uint8_t load_byte(uint32_t address) {
  // I don't THink we need to zero extend anymore
  return read_byte(address);
}

static uint16_t load_halfword(uint32_t address) {
  return read_hword(address);
}

static uint32_t load_word(uint32_t address) {
  return read_word(address);
}

static void store_byte(uint32_t address, uint8_t value) {
  write_byte(address, value);
}

static void store_halfword(uint32_t address, uint16_t value) {
  write_hword(address, value);
}

static void store_word(uint32_t address, uint32_t value) {
  write_word(address, value);
}

static inline uint32_t get_opcode(uint32_t instruction) {
  return instruction >> OPCODE_SHIFT;
}

// ---------------R Type Instructions---------------------
static void add(uint32_t rs, uint32_t rt, uint32_t rd) {
  int32_t lhs = read_gpr(rs);
  int32_t rhs = read_gpr(rt);
  int64_t sum = (int64_t)lhs + (int64_t)rhs;
  uint32_t result = (uint32_t)sum;
  write_gpr(rd, result);
  set_add_flags((uint32_t)lhs, (uint32_t)rhs, result);
}

static void addu(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  uint64_t sum = (uint64_t)lhs + (uint64_t)rhs;
  uint32_t result = (uint32_t)sum;
  write_gpr(rd, result);
  set_add_flags(lhs, rhs, result);
}

static void sub(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  int64_t diff = (int64_t)(int32_t)lhs - (int64_t)(int32_t)rhs;
  uint32_t result = (uint32_t)diff;
  write_gpr(rd, result);
  set_sub_flags(lhs, rhs, result);
}

static void subu(uint32_t rs, uint32_t rt, uint32_t rd) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t rhs = (uint32_t)read_gpr(rt);
  uint64_t diff = (uint64_t)lhs - (uint64_t)rhs;
  uint32_t result = (uint32_t)diff;
  write_gpr(rd, result);
  set_sub_flags(lhs, rhs, result);
}

static void mult(uint32_t rs, uint32_t rt) {
  int32_t lhs = read_gpr(rs);
  int32_t rhs = read_gpr(rt);
  int64_t product = (int64_t)lhs * (int64_t)rhs;
  THE_CPU.hw_registers[HI] = (uint32_t)(product >> 32);
  THE_CPU.hw_registers[LO] = (uint32_t)product;
  set_mul_flags((uint32_t)lhs, (uint32_t)rhs, THE_CPU.hw_registers[LO],
                THE_CPU.hw_registers[HI], true);
}

static void multu(uint32_t rs, uint32_t rt) {
  uint64_t lhs = (uint64_t)(uint32_t)read_gpr(rs);
  uint64_t rhs = (uint64_t)(uint32_t)read_gpr(rt);
  uint64_t product = lhs * rhs;
  THE_CPU.hw_registers[HI] = (uint32_t)(product >> 32);
  THE_CPU.hw_registers[LO] = (uint32_t)product;
  set_mul_flags((uint32_t)lhs, (uint32_t)rhs, THE_CPU.hw_registers[LO],
                THE_CPU.hw_registers[HI], false);
}

static void divv(uint32_t rs, uint32_t rt) {
  int32_t divisor = read_gpr(rt);
  if (divisor == 0) {
    return; // TODO: Throw an exception here
  }
  int32_t dividend = read_gpr(rs);
  uint32_t quotient;
  uint32_t remainder;
  if (dividend == INT32_MIN && divisor == -1) {
    quotient = (uint32_t)INT32_MIN;
    remainder = 0;
  } else {
    quotient = (uint32_t)(dividend / divisor);
    remainder = (uint32_t)(dividend % divisor);
  }
  THE_CPU.hw_registers[LO] = quotient;
  THE_CPU.hw_registers[HI] = remainder;
  set_div_flags((uint32_t)dividend, (uint32_t)divisor, quotient, true);
}

static void divu(uint32_t rs, uint32_t rt) {
  uint32_t divisor = (uint32_t)read_gpr(rt);
  if (divisor == 0) {
    return; // TODO: Throw an exception here
  }
  uint32_t dividend = (uint32_t)read_gpr(rs);
  uint32_t quotient = dividend / divisor;
  uint32_t remainder = dividend % divisor;
  THE_CPU.hw_registers[LO] = quotient;
  THE_CPU.hw_registers[HI] = remainder;
  set_div_flags(dividend, divisor, quotient, false);
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

static void systemcall() {
  uint32_t code = (uint32_t)read_gpr(REG_V0);

  switch (code) {
    case 1: { // print integer in $a0
      int32_t value = read_gpr(REG_A0);
      printf("%d", value);
      fflush(stdout);
      break;
    }
    case 4: { // print string at address in $a0
      uint32_t addr = (uint32_t)read_gpr(REG_A0);
      while (1) {
        uint8_t ch = read_byte(addr++);
        if (ch == 0) break;
        putchar(ch);
      }
      fflush(stdout);
      break;
    }
    case 5: {   // read integer
      int x;
      if (scanf("%d", &x) == 1)
        write_gpr(REG_V0, (uint32_t)x);
      else
      {// EOF or invalid input - clear and return 0
        clearerr(stdin);
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        write_gpr(REG_V0, 0);
      }
      break;
    }
    case 10: {  // exit
      THE_CPU.hw_registers[PC] = CPU_HALT;
      break;
    }
    case 11: {  // clear_screen
      // ANSI clear + cursor home
      printf("\033[2J\033[H");
      fflush(stdout);
      break;
    }
    case 12: {  // read_char_nb
      static struct termios oldt, newt;
      static bool terminal_configured = false;
    
      if (!terminal_configured) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VTIME] = 0;
        newt.c_cc[VMIN] = 0;
        terminal_configured = true;
      }
    
      tcsetattr(STDIN_FILENO, TCSANOW, &newt);
      int ch = getchar();
      tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
      if (ch == EOF) ch = 0;
    
      write_gpr(REG_V0, (uint32_t)ch);
      break;
    }
    case 13: {  // sleep_ms
      uint32_t ms = read_gpr(REG_A0);

      struct timespec ts;
      ts.tv_sec = ms / 1000;
      ts.tv_nsec = (ms % 1000) * 1000000;

      while (nanosleep(&ts, &ts) == -1) {
        // If interrupted by signal, nanosleep updates ts
        continue;
      }
      break;
    }
    case 14: {  // get_time_ms
      struct timeval tv;
      gettimeofday(&tv, NULL);

      uint64_t ms = (uint64_t)tv.tv_sec * 1000ULL
                  + (uint64_t)tv.tv_usec / 1000ULL;

      write_gpr(REG_V0, (uint32_t)ms);   // wrap is fine for MIPS32
      break;
    }
    default:
      fprintf(stderr, "Unhandled syscall code %u\n", code);
      break;
  }
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
    case FUNCT_DIV: divv(rs, rt); break;
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
    case FUNCT_SYSCALL: systemcall(); break;
    case FUNCT_BREAK: breakk(); break;
  }
}

// ---------------I Type Instructions---------------------

static void addi(uint32_t rs, uint32_t rt, uint16_t imm) {
  int32_t simm = sign_extend(imm, 16);
  int32_t lhs = read_gpr(rs);
  int64_t sum = (int64_t)lhs + (int64_t)simm;
  uint32_t result = (uint32_t)sum;
  write_gpr(rt, result);
  set_add_flags((uint32_t)lhs, (uint32_t)simm, result);
}

static void addiu(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t simm = (uint32_t)sign_extend(imm, 16);
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint64_t sum = (uint64_t)lhs + (uint64_t)simm;
  uint32_t result = (uint32_t)sum;
  write_gpr(rt, result);
  set_add_flags(lhs, simm, result);
}

static void andi(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t zimm = zero_extend(imm, 16);
  write_gpr(rt, lhs & zimm);
}

static void ori(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t zimm = zero_extend(imm, 16);
  write_gpr(rt, lhs | zimm);
}

static void xori(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t lhs = (uint32_t)read_gpr(rs);
  uint32_t zimm = zero_extend(imm, 16);
  write_gpr(rt, lhs ^ zimm);
}

static void slti(uint32_t rs, uint32_t rt, uint16_t imm) {
  int32_t simm = sign_extend(imm, 16);
  int32_t lhs = read_gpr(rs);
  write_gpr(rt, lhs < simm ? 1 : 0);
}

static void sltiu(uint32_t rs, uint32_t rt, uint16_t imm) {
  uint32_t simm = (uint32_t)sign_extend(imm, 16);
  uint32_t lhs = (uint32_t)read_gpr(rs);
  write_gpr(rt, lhs < simm ? 1 : 0);
}

static void lui(uint32_t rt, uint16_t imm) {
  uint32_t zimm = zero_extend(imm, 16);
  write_gpr(rt, zimm << 16);
}

static void lw(uint32_t rt, uint32_t effective_address) {
  write_gpr(rt, load_word(effective_address));  
}

static void sw(uint32_t rt, uint32_t effective_address) {
  uint32_t value = (uint32_t)read_gpr(rt);
  store_word(effective_address, value);
}

static void lb(uint32_t rt, uint32_t effective_address) {
  uint8_t raw = load_byte(effective_address);
  int32_t extended = sign_extend(raw, 8);
  write_gpr(rt, extended);
}

static void lbu(uint32_t rt, uint32_t effective_address) {
  uint8_t raw = load_byte(effective_address);
  write_gpr(rt, zero_extend(raw, 8));
}

static void lh(uint32_t rt, uint32_t effective_address) {
  uint16_t raw = load_halfword(effective_address);
  int32_t extended = sign_extend(raw, 16);
  write_gpr(rt, extended);
}

static void lhu(uint32_t rt, uint32_t effective_address) {
  uint16_t raw = load_halfword(effective_address);
  write_gpr(rt, zero_extend(raw, 16));
}

static void sb(uint32_t rt, uint32_t effective_address) {
  uint32_t value = (uint32_t)read_gpr(rt);
  store_byte(effective_address, (uint8_t)(value & 0xFF));
}

static void sh(uint32_t rt, uint32_t effective_address) {
  uint32_t value = (uint32_t)read_gpr(rt);
  store_halfword(effective_address, (uint16_t)(value & 0xFFFF));
}

static void beq(uint32_t rs, uint32_t rt, int32_t offset) {
  if (read_gpr(rs) == read_gpr(rt)) {
    int32_t branch_offset = offset << 2;
    THE_CPU.hw_registers[PC] += (uint32_t)branch_offset;
  }
}

static void bne(uint32_t rs, uint32_t rt, int32_t offset) {
  if (read_gpr(rs) != read_gpr(rt)) {
    int32_t branch_offset = offset << 2;
    THE_CPU.hw_registers[PC] += (uint32_t)branch_offset;
  }
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
  }

  int32_t offset = sign_extend(imm, 16);
  uint32_t base = (uint32_t)read_gpr(rs);
  uint32_t effective_address = base + (uint32_t)offset;
  switch (opcode) {
    case OP_LW: lw(rt, effective_address); return 1;
    case OP_SW: sw(rt, effective_address); return 1;
    case OP_LB: lb(rt, effective_address); return 1;
    case OP_LBU: lbu(rt, effective_address); return 1;
    case OP_LH: lh(rt, effective_address); return 1;
    case OP_LHU: lhu(rt, effective_address); return 1;
    case OP_SB: sb(rt, effective_address); return 1;
    case OP_SH: sh(rt, effective_address); return 1;
    case OP_BEQ: beq(rs, rt, offset); return 1;
    case OP_BNE: bne(rs, rt, offset); return 1;
  }
  return 0;
}


static uint32_t compute_jump_target(uint32_t target) {
  uint32_t pc_plus_4 = THE_CPU.hw_registers[PC] + 4;
  uint32_t upper_bits = pc_plus_4 & 0xF0000000;
  uint32_t target_bits = (target & 0x03FFFFFF) << 2;
  return upper_bits | target_bits;
}

static void j(uint32_t target) {
  THE_CPU.hw_registers[PC] = compute_jump_target(target);
}

static void jal(uint32_t target) {
  uint32_t return_address = THE_CPU.hw_registers[PC] + 4;
  write_gpr(REG_RA, return_address);
  THE_CPU.hw_registers[PC] = compute_jump_target(target);
}

static int handle_jump_instruction(uint32_t instruction) {
  uint32_t opcode = get_opcode(instruction);
  uint32_t target = instruction & 0x01FFFFFF;
  switch (opcode) {
    case OP_J: j(target); return 1;
    case OP_JAL: jal(target); return 1;
  } 
  return 0;
}

static void eret(uint32_t target) {
  // TODO
  (void)target;
}

static int handle_eret_instruction(uint32_t instruction) {
  uint32_t funct = instruction & FUNCT_MASK;
  uint32_t rs = (instruction >> RS_SHIFT) & RS_MASK;
  uint32_t rt = (instruction >> RT_SHIFT) & RT_MASK;
  uint32_t rd = (instruction >> RD_SHIFT) & RD_MASK;
  uint32_t shamt = (instruction >> SHAMT_SHIFT) & SHAMT_MASK;
  if ((rs == 0x10) &&
      (funct == 0x18) &&
      !(rt || rd || shamt)) {
    eret(instruction);
    return 1;
  }
  return 0;
}

static void report_invalid_opcode(const uint32_t opcode, const uint32_t instruction) {
  fprintf(stderr, "ERROR: Invalid opcode %u (IR=0x%04X)\n", (unsigned)opcode,
          (unsigned)instruction);
   THE_CPU.hw_registers[PC] = CPU_HALT;
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
  // J type instructions
  if (handle_jump_instruction(instruction)) {
    return;
  }
  // Special Case
  if ((opcode == OP_ERET) & handle_eret_instruction(instruction)) {
    return;
  // Else: Invalid Opcode
  report_invalid_opcode(opcode, instruction);
  }
}
