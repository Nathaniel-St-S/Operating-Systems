#include "../include/cpu.h"
#include "../include/isa.h"
#include "framework.h"

#include <stdint.h>
#include <string.h>

static uint32_t make_j_instruction(uint32_t opcode, uint32_t target) {
  return (opcode & 0x3F) << OPCODE_SHIFT | (target & 0x03FFFFFF);
}

static void reset_cpu_state(void) {
  memset(&THE_CPU, 0, sizeof(THE_CPU));
}

static uint32_t expected_jump_address(uint32_t pc, uint32_t target) {
  uint32_t pc_plus_4 = pc + 4;
  uint32_t upper_bits = pc_plus_4 & 0xF0000000;
  return upper_bits | ((target & 0x03FFFFFF) << 2);
}

TEST_CASE(Jump, JumpUpdatesPcWithUpperBitsFromPcPlus4) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x12345678;
  uint32_t target = 0x0000123;
  uint32_t instruction = make_j_instruction(OP_J, target);
  execute_instruction(instruction);
  ASSERT_EQ(THE_CPU.hw_registers[PC],
            expected_jump_address(0x12345678, target));
}

TEST_CASE(Jump, JalStoresReturnAddressAndJumps) {
  reset_cpu_state();
  THE_CPU.hw_registers[PC] = 0x0ABCDEF0;
  uint32_t target = 0x0012345;
  uint32_t instruction = make_j_instruction(OP_JAL, target);
  execute_instruction(instruction);
  ASSERT_EQ((uint32_t)read_gpr(REG_RA), (uint32_t)(0x0ABCDEF0 + 4));
  ASSERT_EQ(THE_CPU.hw_registers[PC],
            expected_jump_address(0x0ABCDEF0, target));
}
