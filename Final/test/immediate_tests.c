#include "../include/cpu.h"
#include "../include/isa.h"
#include "framework.h"

#include <stdint.h>
#include <string.h>

static uint32_t make_i_instruction(uint32_t opcode, uint32_t rs, uint32_t rt,
                                   uint32_t immediate) {
  return (opcode & 0x3F) << OPCODE_SHIFT | (rs & 0x1F) << RS_SHIFT |
         (rt & 0x1F) << RT_SHIFT | (immediate & 0xFFFF);
}

static void reset_cpu_state(void) {
  memset(&THE_CPU, 0, sizeof(THE_CPU));
}

TEST_CASE(ITypeImmediate, AddiAddsSignedImmediate) {
  reset_cpu_state();
  write_gpr(REG_T0, 5);
  execute_instruction(make_i_instruction(OP_ADDI, REG_T0, REG_T1, 0xFFFD));
  ASSERT_EQ(read_gpr(REG_T1), 2);
}

TEST_CASE(ITypeImmediate, AddiuWrapsUnsignedResult) {
  reset_cpu_state();
  write_gpr(REG_T0, (uint32_t)0xFFFFFFFF);
  execute_instruction(make_i_instruction(OP_ADDIU, REG_T0, REG_T1, 0x0001));
  ASSERT_EQ((uint32_t)read_gpr(REG_T1), (uint32_t)0x00000000);
}

TEST_CASE(ITypeImmediate, AndiZeroExtendsImmediate) {
  reset_cpu_state();
  write_gpr(REG_T0, (uint32_t)0xF0F0FFFF);
  execute_instruction(make_i_instruction(OP_ANDI, REG_T0, REG_T1, 0x8001));
  ASSERT_EQ((uint32_t)read_gpr(REG_T1), (uint32_t)0x00008001);
}

TEST_CASE(ITypeImmediate, OriCombinesBits) {
  reset_cpu_state();
  write_gpr(REG_T0, 0x00FF0000);
  execute_instruction(make_i_instruction(OP_ORI, REG_T0, REG_T1, 0x1234));
  ASSERT_EQ((uint32_t)read_gpr(REG_T1), (uint32_t)0x00FF1234);
}

TEST_CASE(ITypeImmediate, XoriFlipsBits) {
  reset_cpu_state();
  write_gpr(REG_T0, 0xFFFF0000);
  execute_instruction(make_i_instruction(OP_XORI, REG_T0, REG_T1, 0x0F0F));
  ASSERT_EQ((uint32_t)read_gpr(REG_T1), (uint32_t)0xFFFF0F0F);
}

TEST_CASE(ITypeImmediate, SltiPerformsSignedComparison) {
  reset_cpu_state();
  write_gpr(REG_T0, -2);
  execute_instruction(make_i_instruction(OP_SLTI, REG_T0, REG_T1, 0x0001));
  ASSERT_EQ(read_gpr(REG_T1), 1);
}

TEST_CASE(ITypeImmediate, SltiuPerformsUnsignedComparison) {
  reset_cpu_state();
  write_gpr(REG_T0, (uint32_t)0x80000000);
  execute_instruction(make_i_instruction(OP_SLTIU, REG_T0, REG_T1, 0xFFFF));
  ASSERT_EQ(read_gpr(REG_T1), 1);
}

TEST_CASE(ITypeImmediate, LuiLoadsUpperImmediate) {
  reset_cpu_state();
  execute_instruction(make_i_instruction(OP_LUI, REG_ZERO, REG_T1, 0x1234));
  ASSERT_EQ((uint32_t)read_gpr(REG_T1), (uint32_t)0x12340000);
}
