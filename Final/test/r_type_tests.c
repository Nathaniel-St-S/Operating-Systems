#include "../include/cpu.h"
#include "../include/isa.h"
#include "framework.h"

#include <string.h>

static uword make_r_instruction(word rs, word rt, word rd, word shamt,
                                Funct funct) {
  return ((uword)rs & 0x1F) << 21 | ((uword)rt & 0x1F) << 16 |
         ((uword)rd & 0x1F) << 11 | ((uword)shamt & 0x1F) << 6 |
         ((uword)funct & 0x3F);
}

static void reset_cpu_state(void) {
  memset(&THE_CPU, 0, sizeof(THE_CPU));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0,
                                         FUNCT_MTHI));
  execute_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0,
                                         FUNCT_MTLO));
}

static void set_reg(GP_Register reg, word value) {
  if (reg == REG_ZERO) {
    return;
  }
  THE_CPU.gp_registers[reg] = value;
}

static word get_reg(GP_Register reg) {
  return THE_CPU.gp_registers[reg];
}

static void run_instruction(uword instruction) {
  execute_instruction(instruction);
}

TEST_CASE(RType, Add) {
  reset_cpu_state();
  set_reg(REG_T0, 5);
  set_reg(REG_T1, -3);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_ADD));
  ASSERT_EQ(get_reg(REG_T2), 2);
}

TEST_CASE(RType, AdduWraps) {
  reset_cpu_state();
  set_reg(REG_T0, (word)0xFFFFFFFF);
  set_reg(REG_T1, 1);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_ADDU));
  ASSERT_EQ(get_reg(REG_T2), 0);
}

TEST_CASE(RType, Subtract) {
  reset_cpu_state();
  set_reg(REG_T0, 10);
  set_reg(REG_T1, 4);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SUB));
  ASSERT_EQ(get_reg(REG_T2), 6);
}

TEST_CASE(RType, SubuBorrow) {
  reset_cpu_state();
  set_reg(REG_T0, 0);
  set_reg(REG_T1, 2);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SUBU));
  ASSERT_EQ((uword)get_reg(REG_T2), (uword)0xFFFFFFFE);
}

TEST_CASE(RType, MultProducesHiLo) {
  reset_cpu_state();
  set_reg(REG_T0, 2000);
  set_reg(REG_T1, -3);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_MULT));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ(get_reg(REG_T2), -1);
  ASSERT_EQ(get_reg(REG_T3), -6000);
}

TEST_CASE(RType, MultuProducesHiLo) {
  reset_cpu_state();
  set_reg(REG_T0, (word)0xFFFFFFFF);
  set_reg(REG_T1, 2);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_MULTU));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ((uword)get_reg(REG_T2), (uword)1);
  ASSERT_EQ((uword)get_reg(REG_T3), (uword)0xFFFFFFFE);
}

TEST_CASE(RType, DivSetsQuotientAndRemainder) {
  reset_cpu_state();
  set_reg(REG_T0, 22);
  set_reg(REG_T1, 5);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIV));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ(get_reg(REG_T3), 4);
  ASSERT_EQ(get_reg(REG_T2), 2);
}

TEST_CASE(RType, DivuSetsQuotientAndRemainder) {
  reset_cpu_state();
  set_reg(REG_T0, 25);
  set_reg(REG_T1, 7);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_ZERO, 0, FUNCT_DIVU));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T2, 0, FUNCT_MFHI));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T3, 0, FUNCT_MFLO));
  ASSERT_EQ((uword)get_reg(REG_T3), (uword)3);
  ASSERT_EQ((uword)get_reg(REG_T2), (uword)4);
}

TEST_CASE(RType, MfhiReadsHighRegister) {
  reset_cpu_state();
  set_reg(REG_T0, 0x12345678);
  run_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTHI));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFHI));
  ASSERT_EQ(get_reg(REG_T1), 0x12345678);
}

TEST_CASE(RType, MfloReadsLowRegister) {
  reset_cpu_state();
  set_reg(REG_T0, 0x13579BDF);
  run_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTLO));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFLO));
  ASSERT_EQ(get_reg(REG_T1), 0x13579BDF);
}

TEST_CASE(RType, MthiWritesHighRegister) {
  reset_cpu_state();
  set_reg(REG_T0, 0x7F123456);
  run_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTHI));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFHI));
  ASSERT_EQ(get_reg(REG_T1), 0x7F123456);
}

TEST_CASE(RType, MtloWritesLowRegister) {
  reset_cpu_state();
  set_reg(REG_T0, 0x2468ACE0);
  run_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_MTLO));
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_T1, 0, FUNCT_MFLO));
  ASSERT_EQ(get_reg(REG_T1), 0x2468ACE0);
}

TEST_CASE(RType, AndOperation) {
  reset_cpu_state();
  set_reg(REG_T0, 0xF0F0);
  set_reg(REG_T1, 0x00FF);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_AND));
  ASSERT_EQ(get_reg(REG_T2), 0x00F0);
}

TEST_CASE(RType, OrOperation) {
  reset_cpu_state();
  set_reg(REG_T0, 0x0F00);
  set_reg(REG_T1, 0x00F0);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_OR));
  ASSERT_EQ(get_reg(REG_T2), 0x0FF0);
}

TEST_CASE(RType, XorOperation) {
  reset_cpu_state();
  set_reg(REG_T0, 0xAAAA);
  set_reg(REG_T1, 0x0F0F);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_XOR));
  ASSERT_EQ(get_reg(REG_T2), 0xA5A5);
}

TEST_CASE(RType, NorOperation) {
  reset_cpu_state();
  set_reg(REG_T0, 0xFFFF0000);
  set_reg(REG_T1, 0x0000FFFF);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_NOR));
  ASSERT_EQ((uword)get_reg(REG_T2), (uword)0x00000000);
}

TEST_CASE(RType, ShiftLeftLogicalImmediate) {
  reset_cpu_state();
  set_reg(REG_T1, 1);
  run_instruction(make_r_instruction(REG_ZERO, REG_T1, REG_T2, 4, FUNCT_SLL));
  ASSERT_EQ(get_reg(REG_T2), 16);
}

TEST_CASE(RType, ShiftRightLogicalImmediate) {
  reset_cpu_state();
  set_reg(REG_T1, (word)0xF0);
  run_instruction(make_r_instruction(REG_ZERO, REG_T1, REG_T2, 4, FUNCT_SRL));
  ASSERT_EQ(get_reg(REG_T2), 0x0F);
}

TEST_CASE(RType, ShiftRightArithmeticImmediate) {
  reset_cpu_state();
  set_reg(REG_T1, (word)0xF0000000);
  run_instruction(make_r_instruction(REG_ZERO, REG_T1, REG_T2, 4, FUNCT_SRA));
  ASSERT_EQ((uword)get_reg(REG_T2), (uword)0xFF000000);
}

TEST_CASE(RType, ShiftLeftLogicalVariable) {
  reset_cpu_state();
  set_reg(REG_T0, 3);
  set_reg(REG_T1, 2);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SLLV));
  ASSERT_EQ(get_reg(REG_T2), 16);
}

TEST_CASE(RType, ShiftRightLogicalVariable) {
  reset_cpu_state();
  set_reg(REG_T0, 2);
  set_reg(REG_T1, 0x40);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SRLV));
  ASSERT_EQ(get_reg(REG_T2), 0x10);
}

TEST_CASE(RType, ShiftRightArithmeticVariable) {
  reset_cpu_state();
  set_reg(REG_T0, 3);
  set_reg(REG_T1, (word)0xF0000000);
  run_instruction(make_r_instruction(REG_T0, REG_T1, REG_T2, 0, FUNCT_SRAV));
  ASSERT_EQ((uword)get_reg(REG_T2), (uword)0xFE000000);
}

TEST_CASE(RType, JumpRegisterUpdatesPC) {
  reset_cpu_state();
  set_reg(REG_T0, 0x100);
  run_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_ZERO, 0, FUNCT_JR));
  ASSERT_EQ(THE_CPU.hw_registers[PC], 0x100);
}

TEST_CASE(RType, JalrStoresReturnAddress) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x40;
  set_reg(REG_T0, 0x200);
  run_instruction(make_r_instruction(REG_T0, REG_ZERO, REG_RA, 0, FUNCT_JALR));
  ASSERT_EQ(get_reg(REG_RA), 0x40);
  ASSERT_EQ(THE_CPU.hw_registers[PC], 0x200);
}

TEST_CASE(RType, SyscallHaltsCpu) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x10;
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0, FUNCT_SYSCALL));
  ASSERT_EQ(THE_CPU.hw_registers[PC], (word)CPU_HALT);
}

TEST_CASE(RType, BreakHaltsCpu) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x20;
  run_instruction(make_r_instruction(REG_ZERO, REG_ZERO, REG_ZERO, 0, FUNCT_BREAK));
  ASSERT_EQ(THE_CPU.hw_registers[PC], (word)CPU_HALT);
}
