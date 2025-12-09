#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/isa.h"
#include "framework.h"

#include <stdint.h>
#include <string.h>

static void reset_cpu_and_memory(void) {
  memset(&THE_CPU, 0, sizeof(Cpu));
  free_memory();
  init_memory(CACHE_WRITE_THROUGH);
  set_current_process(SYSTEM_PROCESS_ID);
}

// ============================================
// CPU Initialization Tests
// ============================================

TEST_CASE(CPU, InitializationSetsPC) {
  reset_cpu_and_memory();
  init_cpu(0x1000);
  ASSERT_EQ(HW_REGISTER(PC), 0x1000);
}

TEST_CASE(CPU, InitializationSetsZeroFlag) {
  reset_cpu_and_memory();
  init_cpu(0x2000);
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_ZERO);
}

TEST_CASE(CPU, InitializationClearsRegisters) {
  reset_cpu_and_memory();
  init_cpu(0x3000);
  for (int i = 1; i < GP_REG_COUNT; i++) {
    ASSERT_EQ(GP_REGISTER(i), 0);
  }
}

TEST_CASE(CPU, InitializationSetsZeroRegister) {
  reset_cpu_and_memory();
  init_cpu(0x4000);
  ASSERT_EQ(GP_REGISTER(REG_ZERO), 0);
}

// ============================================
// Fetch Tests
// ============================================

TEST_CASE(CPU, FetchLoadsInstructionFromMemory) {
  reset_cpu_and_memory();
  uint32_t test_addr = 0x1000;
  uint32_t test_instruction = 0x12345678;
  
  write_word(test_addr, test_instruction);
  init_cpu(test_addr);
  fetch();
  
  ASSERT_EQ(HW_REGISTER(IR), test_instruction);
}

TEST_CASE(CPU, FetchIncrementsPC) {
  reset_cpu_and_memory();
  uint32_t start_addr = 0x2000;
  write_word(start_addr, 0xAAAAAAAA);
  
  init_cpu(start_addr);
  fetch();
  
  ASSERT_EQ(HW_REGISTER(PC), start_addr + 4);
}

TEST_CASE(CPU, FetchMultipleInstructions) {
  reset_cpu_and_memory();
  uint32_t base_addr = 0x3000;
  
  write_word(base_addr, 0x11111111);
  write_word(base_addr + 4, 0x22222222);
  write_word(base_addr + 8, 0x33333333);
  
  init_cpu(base_addr);
  
  fetch();
  ASSERT_EQ(HW_REGISTER(IR), 0x11111111);
  ASSERT_EQ(HW_REGISTER(PC), base_addr + 4);
  
  fetch();
  ASSERT_EQ(HW_REGISTER(IR), 0x22222222);
  ASSERT_EQ(HW_REGISTER(PC), base_addr + 8);
  
  fetch();
  ASSERT_EQ(HW_REGISTER(IR), 0x33333333);
  ASSERT_EQ(HW_REGISTER(PC), base_addr + 12);
}

// ============================================
// Register Access Tests
// ============================================

TEST_CASE(CPU, ZeroRegisterAlwaysReadsZero) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_ZERO) = 0x12345678;
  ASSERT_EQ(GP_REGISTER(REG_ZERO), 0);
}

TEST_CASE(CPU, GeneralPurposeRegisterReadWrite) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_T0) = 0xDEADBEEF;
  ASSERT_EQ(GP_REGISTER(REG_T0), 0xDEADBEEF);
}

TEST_CASE(CPU, AllGPRegistersIndependent) {
  reset_cpu_and_memory();
  for (int i = 1; i < GP_REG_COUNT; i++) {
    GP_REGISTER(i) = i * 0x11111111;
  }
  for (int i = 1; i < GP_REG_COUNT; i++) {
    ASSERT_EQ(GP_REGISTER(i), (uint32_t)(i * 0x11111111));
  }
}

TEST_CASE(CPU, HardwareRegisterAccess) {
  reset_cpu_and_memory();
  HW_REGISTER(PC) = 0x5000;
  HW_REGISTER(IR) = 0xABCDEF12;
  HW_REGISTER(FLAGS) = F_ZERO | F_CARRY;
  
  ASSERT_EQ(HW_REGISTER(PC), 0x5000);
  ASSERT_EQ(HW_REGISTER(IR), 0xABCDEF12);
  ASSERT_EQ(HW_REGISTER(FLAGS), (uint32_t)(F_ZERO | F_CARRY));
}

TEST_CASE(CPU, StackPointerRegister) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_SP) = 0x7FFFFFFC;
  ASSERT_EQ(GP_REGISTER(REG_SP), 0x7FFFFFFC);
  
  GP_REGISTER(REG_SP) -= 4;
  ASSERT_EQ(GP_REGISTER(REG_SP), 0x7FFFFFF8);
}

TEST_CASE(CPU, ReturnAddressRegister) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_RA) = 0x1234;
  ASSERT_EQ(GP_REGISTER(REG_RA), 0x1234);
}

// ============================================
// Flag Tests
// ============================================

TEST_CASE(CPU, ZeroFlagSetAndClear) {
  reset_cpu_and_memory();
  HW_REGISTER(FLAGS) = 0;
  HW_REGISTER(FLAGS) |= F_ZERO;
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_ZERO);
  
  HW_REGISTER(FLAGS) &= ~F_ZERO;
  ASSERT_TRUE(!(HW_REGISTER(FLAGS) & F_ZERO));
}

TEST_CASE(CPU, CarryFlagSetAndClear) {
  reset_cpu_and_memory();
  HW_REGISTER(FLAGS) = 0;
  HW_REGISTER(FLAGS) |= F_CARRY;
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_CARRY);
  
  HW_REGISTER(FLAGS) &= ~F_CARRY;
  ASSERT_TRUE(!(HW_REGISTER(FLAGS) & F_CARRY));
}

TEST_CASE(CPU, OverflowFlagSetAndClear) {
  reset_cpu_and_memory();
  HW_REGISTER(FLAGS) = 0;
  HW_REGISTER(FLAGS) |= F_OVERFLOW;
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_OVERFLOW);
  
  HW_REGISTER(FLAGS) &= ~F_OVERFLOW;
  ASSERT_TRUE(!(HW_REGISTER(FLAGS) & F_OVERFLOW));
}

TEST_CASE(CPU, MultipleFlagsSimultaneous) {
  reset_cpu_and_memory();
  HW_REGISTER(FLAGS) = F_ZERO | F_CARRY | F_OVERFLOW;
  
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_ZERO);
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_CARRY);
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_OVERFLOW);
}

TEST_CASE(CPU, ClearAllFlags) {
  reset_cpu_and_memory();
  HW_REGISTER(FLAGS) = F_ZERO | F_CARRY | F_OVERFLOW;
  HW_REGISTER(FLAGS) = 0;
  
  ASSERT_EQ(HW_REGISTER(FLAGS), 0);
}

// ============================================
// HI/LO Register Tests
// ============================================

TEST_CASE(CPU, HIRegisterAccess) {
  reset_cpu_and_memory();
  HW_REGISTER(HI) = 0x12345678;
  ASSERT_EQ(HW_REGISTER(HI), 0x12345678);
}

TEST_CASE(CPU, LORegisterAccess) {
  reset_cpu_and_memory();
  HW_REGISTER(LO) = 0xABCDEF00;
  ASSERT_EQ(HW_REGISTER(LO), 0xABCDEF00);
}

TEST_CASE(CPU, HILOIndependentAccess) {
  reset_cpu_and_memory();
  HW_REGISTER(HI) = 0x11111111;
  HW_REGISTER(LO) = 0x22222222;
  
  ASSERT_EQ(HW_REGISTER(HI), 0x11111111);
  ASSERT_EQ(HW_REGISTER(LO), 0x22222222);
}

// ============================================
// Fetch-Execute Cycle Integration Tests
// ============================================

TEST_CASE(CPU, SimpleFetchExecuteCycle) {
  reset_cpu_and_memory();
  uint32_t addr = 0x1000;
  
  // NOP instruction (sll $zero, $zero, 0)
  uint32_t nop = 0x00000000;
  write_word(addr, nop);
  
  init_cpu(addr);
  fetch();
  execute();
  
  ASSERT_EQ(HW_REGISTER(PC), addr + 4);
}

TEST_CASE(CPU, ExecutePreservesNonTargetRegisters) {
  reset_cpu_and_memory();
  
  // Set up registers
  GP_REGISTER(REG_T0) = 0xAAAAAAAA;
  GP_REGISTER(REG_T1) = 0xBBBBBBBB;
  GP_REGISTER(REG_T2) = 0xCCCCCCCC;
  
  // Execute NOP
  HW_REGISTER(IR) = 0x00000000;
  execute();
  
  // T0, T1, T2 should be unchanged
  ASSERT_EQ(GP_REGISTER(REG_T0), 0xAAAAAAAA);
  ASSERT_EQ(GP_REGISTER(REG_T1), 0xBBBBBBBB);
  ASSERT_EQ(GP_REGISTER(REG_T2), 0xCCCCCCCC);
}

// ============================================
// Memory Access Through CPU Tests
// ============================================

TEST_CASE(CPU, CPUMemoryReadWrite) {
  reset_cpu_and_memory();
  uint32_t addr = 0x2000;
  uint32_t value = 0x12345678;
  
  write_word(addr, value);
  ASSERT_EQ(read_word(addr), value);
}

TEST_CASE(CPU, InstructionFetchFromDifferentAddresses) {
  reset_cpu_and_memory();
  
  write_word(0x1000, 0x11111111);
  write_word(0x2000, 0x22222222);
  write_word(0x3000, 0x33333333);
  
  init_cpu(0x1000);
  fetch();
  ASSERT_EQ(HW_REGISTER(IR), 0x11111111);
  
  HW_REGISTER(PC) = 0x2000;
  fetch();
  ASSERT_EQ(HW_REGISTER(IR), 0x22222222);
  
  HW_REGISTER(PC) = 0x3000;
  fetch();
  ASSERT_EQ(HW_REGISTER(IR), 0x33333333);
}

// ============================================
// Register Naming Tests
// ============================================

TEST_CASE(CPU, ArgumentRegisters) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_A0) = 1;
  GP_REGISTER(REG_A1) = 2;
  GP_REGISTER(REG_A2) = 3;
  GP_REGISTER(REG_A3) = 4;
  
  ASSERT_EQ(GP_REGISTER(REG_A0), 1);
  ASSERT_EQ(GP_REGISTER(REG_A1), 2);
  ASSERT_EQ(GP_REGISTER(REG_A2), 3);
  ASSERT_EQ(GP_REGISTER(REG_A3), 4);
}

TEST_CASE(CPU, ReturnValueRegisters) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_VO) = 0x100;
  GP_REGISTER(REG_V1) = 0x200;
  
  ASSERT_EQ(GP_REGISTER(REG_VO), 0x100);
  ASSERT_EQ(GP_REGISTER(REG_V1), 0x200);
}

TEST_CASE(CPU, TemporaryRegisters) {
  reset_cpu_and_memory();
  for (int i = REG_T0; i <= REG_T7; i++) {
    GP_REGISTER(i) = i * 0x10;
  }
  for (int i = REG_T0; i <= REG_T7; i++) {
    ASSERT_EQ(GP_REGISTER(i), (uint32_t)(i * 0x10));
  }
}

TEST_CASE(CPU, SavedRegisters) {
  reset_cpu_and_memory();
  for (int i = REG_S0; i <= REG_S7; i++) {
    GP_REGISTER(i) = i * 0x100;
  }
  for (int i = REG_S0; i <= REG_S7; i++) {
    ASSERT_EQ(GP_REGISTER(i), (uint32_t)(i * 0x100));
  }
}

// ============================================
// Edge Cases
// ============================================

TEST_CASE(CPU, PCWrapAround) {
  reset_cpu_and_memory();
  HW_REGISTER(PC) = 0xFFFFFFFC;
  // This would wrap on increment, but we test the value itself
  ASSERT_EQ(HW_REGISTER(PC), 0xFFFFFFFC);
}

TEST_CASE(CPU, MaximumRegisterValue) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_T0) = 0xFFFFFFFF;
  ASSERT_EQ(GP_REGISTER(REG_T0), 0xFFFFFFFF);
}

TEST_CASE(CPU, MinimumRegisterValue) {
  reset_cpu_and_memory();
  GP_REGISTER(REG_T0) = 0x00000000;
  ASSERT_EQ(GP_REGISTER(REG_T0), 0x00000000);
}

// ============================================
// State Consistency Tests
// ============================================

TEST_CASE(CPU, RepeatedInitialization) {
  reset_cpu_and_memory();
  
  init_cpu(0x1000);
  ASSERT_EQ(HW_REGISTER(PC), 0x1000);
  
  init_cpu(0x2000);
  ASSERT_EQ(HW_REGISTER(PC), 0x2000);
  
  init_cpu(0x3000);
  ASSERT_EQ(HW_REGISTER(PC), 0x3000);
}

TEST_CASE(CPU, StateAfterMultipleOperations) {
  reset_cpu_and_memory();
  
  init_cpu(0x1000);
  GP_REGISTER(REG_T0) = 42;
  HW_REGISTER(FLAGS) = F_ZERO;
  
  ASSERT_EQ(HW_REGISTER(PC), 0x1000);
  ASSERT_EQ(GP_REGISTER(REG_T0), 42);
  ASSERT_TRUE(HW_REGISTER(FLAGS) & F_ZERO);
}

// ============================================
// MAR and MBR Tests
// ============================================

TEST_CASE(CPU, MemoryAddressRegister) {
  reset_cpu_and_memory();
  HW_REGISTER(MAR) = 0x5000;
  ASSERT_EQ(HW_REGISTER(MAR), 0x5000);
}

TEST_CASE(CPU, MemoryBufferRegister) {
  reset_cpu_and_memory();
  HW_REGISTER(MBR) = 0xDEADBEEF;
  ASSERT_EQ(HW_REGISTER(MBR), 0xDEADBEEF);
}

// ============================================
// I/O Register Tests
// ============================================

TEST_CASE(CPU, IOAddressRegister) {
  reset_cpu_and_memory();
  HW_REGISTER(IO_AR) = 0x100;
  ASSERT_EQ(HW_REGISTER(IO_AR), 0x100);
}

TEST_CASE(CPU, IOBufferRegister) {
  reset_cpu_and_memory();
  HW_REGISTER(IO_BR) = 0xFF;
  ASSERT_EQ(HW_REGISTER(IO_BR), 0xFF);
}
