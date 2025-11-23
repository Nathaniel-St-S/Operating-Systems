#include "../include/cpu.h"
#include "../include/isa.h"
#include "../include/memory.h"
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

static void ensure_memory_initialized(void) {
  static int initialized = 0;
  if (!initialized) {
    init_cache(&L1, L1CACHE_SIZE);
    init_cache(&L2, L2CACHE_SIZE);
    init_ram(RAM_SIZE);
    initialized = 1;
  }
}

static void reset_cache_state(Cache *cache) {
  if (cache->items == NULL) {
    return;
  }
  for (int i = 0; i < cache->size; ++i) {
    cache->items[i].addr = EMPTY_ADDR;
    cache->items[i].val = NO_VAL;
  }
  cache->front = 0;
  cache->count = 0;
}

static void reset_memory_state(void) {
  ensure_memory_initialized();
  for (int i = 0; i < RAM_SIZE; ++i) {
    RAM[i] = 0;
  }
  reset_cache_state(&L1);
  reset_cache_state(&L2);
}

static void reset_cpu_and_memory(void) {
  reset_cpu_state();
  reset_memory_state();
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

TEST_CASE(ITypeImmediate, LoadWordFetchesStoredValue) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 100);
  write_gpr(REG_T1, 0x12345678);
  execute_instruction(make_i_instruction(OP_SW, REG_T0, REG_T1, 0));
  write_gpr(REG_T1, 0);
  execute_instruction(make_i_instruction(OP_LW, REG_T0, REG_T1, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T1), (uint32_t)0x12345678);
}

TEST_CASE(ITypeImmediate, LoadByteSignExtendsValue) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 120);
  write_gpr(REG_T1, 0x000000FF);
  execute_instruction(make_i_instruction(OP_SB, REG_T0, REG_T1, 0));
  write_gpr(REG_T2, 0);
  execute_instruction(make_i_instruction(OP_LB, REG_T0, REG_T2, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0xFFFFFFFF);
}

TEST_CASE(ITypeImmediate, LoadByteUnsignedZeroExtends) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 140);
  write_gpr(REG_T1, 0x00000080);
  execute_instruction(make_i_instruction(OP_SB, REG_T0, REG_T1, 0));
  write_gpr(REG_T2, 0);
  execute_instruction(make_i_instruction(OP_LBU, REG_T0, REG_T2, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0x00000080);
}

TEST_CASE(ITypeImmediate, LoadHalfSignExtendsValue) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 160);
  write_gpr(REG_T1, 0x0000F234);
  execute_instruction(make_i_instruction(OP_SH, REG_T0, REG_T1, 0));
  write_gpr(REG_T2, 0);
  execute_instruction(make_i_instruction(OP_LH, REG_T0, REG_T2, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0xFFFFF234);
}

TEST_CASE(ITypeImmediate, LoadHalfUnsignedZeroExtends) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 180);
  write_gpr(REG_T1, 0x0000ABCD);
  execute_instruction(make_i_instruction(OP_SH, REG_T0, REG_T1, 0));
  write_gpr(REG_T2, 0);
  execute_instruction(make_i_instruction(OP_LHU, REG_T0, REG_T2, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0x0000ABCD);
}

TEST_CASE(ITypeImmediate, StoreByteUpdatesSingleByte) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 200);
  write_gpr(REG_T1, 0x11223344);
  execute_instruction(make_i_instruction(OP_SW, REG_T0, REG_T1, 0));
  write_gpr(REG_T2, 0x000000AA);
  execute_instruction(make_i_instruction(OP_SB, REG_T0, REG_T2, 2));
  write_gpr(REG_T3, 0);
  execute_instruction(make_i_instruction(OP_LW, REG_T0, REG_T3, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T3), (uint32_t)0x11AA3344);
}

TEST_CASE(ITypeImmediate, StoreHalfWritesTwoBytes) {
  reset_cpu_and_memory();
  write_gpr(REG_T0, 220);
  write_gpr(REG_T1, 0x00001234);
  execute_instruction(make_i_instruction(OP_SH, REG_T0, REG_T1, 0));
  write_gpr(REG_T2, 0);
  execute_instruction(make_i_instruction(OP_LW, REG_T0, REG_T2, 0));
  ASSERT_EQ((uint32_t)read_gpr(REG_T2), (uint32_t)0x00001234);
}

TEST_CASE(ITypeImmediate, BeqBranchesWhenEqual) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x100;
  write_gpr(REG_T0, 7);
  write_gpr(REG_T1, 7);
  execute_instruction(make_i_instruction(OP_BEQ, REG_T0, REG_T1, 0x0002));
  ASSERT_EQ(THE_CPU.hw_registers[PC], (uint32_t)(0x100 + 4 + (2 << 2)));
}

TEST_CASE(ITypeImmediate, BneBranchesWhenNotEqual) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x80;
  write_gpr(REG_T0, 1);
  write_gpr(REG_T1, 2);
  execute_instruction(make_i_instruction(OP_BNE, REG_T0, REG_T1, 0x0003));
  ASSERT_EQ(THE_CPU.hw_registers[PC], (uint32_t)(0x80 + 4 + (3 << 2)));
}
