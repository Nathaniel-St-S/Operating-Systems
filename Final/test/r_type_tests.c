#include "../include/cpu.h"
#include "../include/isa.h"
#include "framework.h"

#include <stdint.h>
#include <string.h>
#include <limits.h>

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


static uint32_t make_r_instruction(uint32_t rs, uint32_t rt, uint32_t rd,
                                   uint32_t shamt, uint32_t funct) {
  return (rs & 0x1F) << RS_SHIFT | (rt & 0x1F) << RT_SHIFT | (rd & 0x1F) << RD_SHIFT |
         (shamt & 0x1F) << SHAMT_SHIFT | (funct & 0x3F);
}

static void reset_cpu_state(void) {
  memset(&THE_CPU, 0, sizeof(THE_CPU));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0,
                                         FUNCT_MTHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0,
                                         FUNCT_MTLO));
}

TEST_CASE(RType, Add) {
  reset_cpu_state();
  write_gpr(REG_T0, 5);
  write_gpr(REG_T1, -3);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_ADD));
  ASSERT_EQ(read_gpr(REG_T2), 2);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
}

TEST_CASE(RType, AdduWraps) {
  reset_cpu_state();
  write_gpr(REG_T0, (uint32_t)0xFFFFFFFF);
  write_gpr(REG_T1, 1);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_ADDU));
  ASSERT_EQ(read_gpr(REG_T2), 0);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_CARRY);
}

TEST_CASE(RType, ZeroFlagSetOnZeroResult) {
  reset_cpu_state();
  write_gpr(REG_T0, 42);
  write_gpr(REG_T1, 42);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SUB));
  ASSERT_EQ(read_gpr(REG_T2), 0);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_ZERO);
}

TEST_CASE(RType, CarryFlagNotSetWithoutWrap) {
  reset_cpu_state();
  write_gpr(REG_T0, 1);
  write_gpr(REG_T1, 1);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_ADDU));
  ASSERT_EQ(read_gpr(REG_T2), 2);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
}

TEST_CASE(RType, OverflowFlagSetOnSignedAdd) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x7FFFFFFF);
  write_gpr(REG_T1, 1);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_ADD));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0x80000000);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW);
}

TEST_CASE(RType, Subtract) {
  reset_cpu_state();
  write_gpr(REG_T0, 10);
  write_gpr(REG_T1, 4);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SUB));
  ASSERT_EQ(read_gpr(REG_T2), 6);
}

TEST_CASE(RType, SubuBorrow) {
  reset_cpu_state();
  write_gpr(REG_T0, 0);
  write_gpr(REG_T1, 2);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SUBU));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0xFFFFFFFE);
}

TEST_CASE(RType, MultProducesHiLo) {
  reset_cpu_state();
  write_gpr(REG_T0, 2000);
  write_gpr(REG_T1, -3);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_MULT));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ(read_gpr(REG_T2), -1);
  ASSERT_EQ(read_gpr(REG_T3), -6000);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_CARRY);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
}

TEST_CASE(RType, MultuProducesHiLo) {
  reset_cpu_state();
  write_gpr(REG_T0, (uint32_t)0xFFFFFFFF);
  write_gpr(REG_T1, 2);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_MULTU));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)1);
  ASSERT_EQ((uint32_t)read_gpr(REG_T3), (uint32_t)0xFFFFFFFE);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_CARRY);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
}

TEST_CASE(RType, MultZeroResultSetsZeroFlag) {
  reset_cpu_state();
  write_gpr(REG_T0, 0);
  write_gpr(REG_T1, 12345);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_MULT));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFLO));
  ASSERT_EQ(read_gpr(REG_T2), 0);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_ZERO);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
}

TEST_CASE(RType, MultOverflowSetsCarryAndOverflow) {
  reset_cpu_state();
  write_gpr(REG_T0, INT32_MAX);
  write_gpr(REG_T1, 4);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_MULT));
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_CARRY);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
}

TEST_CASE(RType, DivSetsQuotientAndRemainder) {
  reset_cpu_state();
  write_gpr(REG_T0, 22);
  write_gpr(REG_T1, 5);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIV));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ(read_gpr(REG_T3), 4);
  ASSERT_EQ(read_gpr(REG_T2), 2);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
}

TEST_CASE(RType, DivuSetsQuotientAndRemainder) {
  reset_cpu_state();
  write_gpr(REG_T0, 25);
  write_gpr(REG_T1, 7);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIVU));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ((uint32_t)read_gpr(REG_T3), (uint32_t)3);
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)4);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
}

TEST_CASE(RType, DivZeroQuotientSetsZeroFlag) {
  reset_cpu_state();
  write_gpr(REG_T0, 1);
  write_gpr(REG_T1, 2);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIV));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFLO));
  ASSERT_EQ(read_gpr(REG_T2), 0);
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_ZERO);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
}

TEST_CASE(RType, DivOverflowSetsOverflowFlag) {
  reset_cpu_state();
  write_gpr(REG_T0, INT32_MIN);
  write_gpr(REG_T1, -1);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIV));
  ASSERT_TRUE(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW);
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_ZERO));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
}

TEST_CASE(RType, DivuClearsCarryFlag) {
  reset_cpu_state();
  THE_CPU.hw_registers[FLAGS] = F_CARRY;
  write_gpr(REG_T0, 30);
  write_gpr(REG_T1, 5);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIVU));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_CARRY));
  ASSERT_TRUE(!(THE_CPU.hw_registers[FLAGS] & F_OVERFLOW));
}

TEST_CASE(RType, MfhiReadsHighRegister) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x12345678);
  execute_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFHI));
  ASSERT_EQ(read_gpr(REG_T1), 0x12345678);
}

TEST_CASE(RType, MfloReadsLowRegister) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x13579BDF);
  execute_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTLO));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFLO));
  ASSERT_EQ(read_gpr(REG_T1), 0x13579BDF);
}

TEST_CASE(RType, MthiWritesHighRegister) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x7F123456);
  execute_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFHI));
  ASSERT_EQ(read_gpr(REG_T1), 0x7F123456);
}

TEST_CASE(RType, MtloWritesLowRegister) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x2468ACE0);
  execute_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTLO));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFLO));
  ASSERT_EQ(read_gpr(REG_T1), 0x2468ACE0);
}

TEST_CASE(RType, AndOperation) {
  reset_cpu_state();
  write_gpr(REG_T0, 0xF0F0);
  write_gpr(REG_T1, 0x00FF);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_AND));
  ASSERT_EQ(read_gpr(REG_T2), 0x00F0);
}

TEST_CASE(RType, OrOperation) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x0F00);
  write_gpr(REG_T1, 0x00F0);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_OR));
  ASSERT_EQ(read_gpr(REG_T2), 0x0FF0);
}

TEST_CASE(RType, XorOperation) {
  reset_cpu_state();
  write_gpr(REG_T0, 0xAAAA);
  write_gpr(REG_T1, 0x0F0F);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_XOR));
  ASSERT_EQ(read_gpr(REG_T2), 0xA5A5);
}

TEST_CASE(RType, NorOperation) {
  reset_cpu_state();
  write_gpr(REG_T0, 0xFFFF0000);
  write_gpr(REG_T1, 0x0000FFFF);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_NOR));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0x00000000);
}

TEST_CASE(RType, ShiftLeftLogicalImmediate) {
  reset_cpu_state();
  write_gpr(REG_T1, 1);
  execute_instruction(make_r_instruction(REG_ZERO, REG_T1, REG_T2, 4, FUNCT_SLL));
  ASSERT_EQ(read_gpr(REG_T2), 16);
}

TEST_CASE(RType, ShiftRightLogicalImmediate) {
  reset_cpu_state();
  write_gpr(REG_T1, (uint32_t)0xF0);
  execute_instruction(make_r_instruction(REG_ZERO, REG_T1, REG_T2, 4, FUNCT_SRL));
  ASSERT_EQ(read_gpr(REG_T2), 0x0F);
}

TEST_CASE(RType, ShiftRightArithmeticImmediate) {
  reset_cpu_state();
  write_gpr(REG_T1, (uint32_t)0xF0000000);
  execute_instruction(make_r_instruction(REG_ZERO, REG_T1, REG_T2, 4, FUNCT_SRA));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0xFF000000);
}

TEST_CASE(RType, ShiftLeftLogicalVariable) {
  reset_cpu_state();
  write_gpr(REG_T0, 3);
  write_gpr(REG_T1, 2);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SLLV));
  ASSERT_EQ(read_gpr(REG_T2), 16);
}

TEST_CASE(RType, ShiftRightLogicalVariable) {
  reset_cpu_state();
  write_gpr(REG_T0, 2);
  write_gpr(REG_T1, 0x40);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SRLV));
  ASSERT_EQ(read_gpr(REG_T2), 0x10);
}

TEST_CASE(RType, ShiftRightArithmeticVariable) {
  reset_cpu_state();
  write_gpr(REG_T0, 3);
  write_gpr(REG_T1, 0xF0000000);
  execute_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SRAV));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), 0xFE000000);
}

TEST_CASE(RType, JumpRegisterUpdatesPC) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x100);
  execute_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_JR));
  ASSERT_EQ(THE_CPU.hw_registers[PC], 0x100);
}

TEST_CASE(RType, JalrStoresReturnAddress) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x40;
  write_gpr(REG_T0, 0x200);
  execute_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_RA, 0, FUNCT_JALR));
  ASSERT_EQ(read_gpr(REG_RA), 0x40);
  ASSERT_EQ(THE_CPU.hw_registers[PC], 0x200);
}

TEST_CASE(RType, SyscallHaltsCpu) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x10;
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0, FUNCT_SYSCALL));
  ASSERT_EQ(THE_CPU.hw_registers[PC], CPU_HALT);
}

TEST_CASE(RType, BreakHaltsCpu) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x20;
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0, FUNCT_BREAK));
  ASSERT_EQ(THE_CPU.hw_registers[PC], CPU_HALT);
}
