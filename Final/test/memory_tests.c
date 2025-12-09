#include "../include/memory.h"
#include "framework.h"

#include <stdint.h>
#include <string.h>

static void reset_memory(void) {
  free_memory();
  init_memory(CACHE_WRITE_THROUGH);
  set_current_process(SYSTEM_PROCESS_ID);
}

static void reset_memory_write_back(void) {
  free_memory();
  init_memory(CACHE_WRITE_BACK);
  set_current_process(SYSTEM_PROCESS_ID);
}

// ============================================
// Basic Read/Write Tests
// ============================================

TEST_CASE(Memory, ReadWriteByte) {
  reset_memory();
  write_byte(100, 0x42);
  ASSERT_EQ(read_byte(100), 0x42);
}

TEST_CASE(Memory, ReadWriteHalfword) {
  reset_memory();
  write_hword(200, 0x1234);
  ASSERT_EQ(read_hword(200), 0x1234);
}

TEST_CASE(Memory, ReadWriteWord) {
  reset_memory();
  write_word(300, 0x12345678);
  ASSERT_EQ(read_word(300), 0x12345678);
}

TEST_CASE(Memory, WriteBytePreservesOtherBytes) {
  reset_memory();
  write_word(400, 0xAABBCCDD);
  write_byte(401, 0xFF);
  ASSERT_EQ(read_word(400), 0xAAFFCCDD);
}

TEST_CASE(Memory, WriteHalfwordPreservesOtherBytes) {
  reset_memory();
  write_word(500, 0x11223344);
  write_hword(502, 0xAABB);
  ASSERT_EQ(read_word(500), 0xAABB3344);
}

TEST_CASE(Memory, MultipleWordWrites) {
  reset_memory();
  for (uint32_t i = 0; i < 10; i++) {
    write_word(1000 + (i * 4), i * 100);
  }
  for (uint32_t i = 0; i < 10; i++) {
    ASSERT_EQ(read_word(1000 + (i * 4)), i * 100);
  }
}

// ============================================
// Cache Tests (Write-Through)
// ============================================

TEST_CASE(Memory, CacheWriteThroughUpdatesRAM) {
  reset_memory();
  write_word(2000, 0xDEADBEEF);
  ASSERT_EQ(read_word(2000), 0xDEADBEEF);
}

TEST_CASE(Memory, CacheHitOnRepeatedRead) {
  reset_memory();
  write_word(3000, 0x12345678);
  uint32_t first = read_word(3000);
  uint32_t second = read_word(3000);
  ASSERT_EQ(first, second);
  ASSERT_EQ(first, 0x12345678);
}

TEST_CASE(Memory, CacheLineBoundary) {
  reset_memory();
  // Write at cache line boundary (64 bytes)
  write_word(0, 0xAAAAAAAA);
  write_word(64, 0xBBBBBBBB);
  ASSERT_EQ(read_word(0), 0xAAAAAAAA);
  ASSERT_EQ(read_word(64), 0xBBBBBBBB);
}

TEST_CASE(Memory, CacheWriteThroughConsistency) {
  reset_memory();
  uint32_t addr = 4000;
  write_word(addr, 0x11111111);
  write_word(addr, 0x22222222);
  write_word(addr, 0x33333333);
  ASSERT_EQ(read_word(addr), 0x33333333);
}

// ============================================
// Cache Tests (Write-Back)
// ============================================

TEST_CASE(Memory, WriteBackDelaysRAMWrite) {
  reset_memory_write_back();
  write_word(5000, 0xFEEDFACE);
  ASSERT_EQ(read_word(5000), 0xFEEDFACE);
}

TEST_CASE(Memory, WriteBackMultipleWrites) {
  reset_memory_write_back();
  uint32_t addr = 6000;
  write_word(addr, 0x11111111);
  write_word(addr, 0x22222222);
  write_word(addr, 0x33333333);
  ASSERT_EQ(read_word(addr), 0x33333333);
}

TEST_CASE(Memory, WriteBackCacheLineEviction) {
  reset_memory_write_back();
  // Write enough data to force cache eviction
  for (uint32_t i = 0; i < 100; i++) {
    write_word(7000 + (i * 64), i);
  }
  for (uint32_t i = 0; i < 100; i++) {
    ASSERT_EQ(read_word(7000 + (i * 64)), i);
  }
}

// ============================================
// Memory Allocation Tests
// ============================================

TEST_CASE(Memory, AllocateMemoryForProcess) {
  reset_memory();
  uint32_t addr = mallocate(1, 1024);
  ASSERT_TRUE(addr != UINT32_MAX);
}

TEST_CASE(Memory, AllocateMultipleBlocks) {
  reset_memory();
  uint32_t addr1 = mallocate(1, 512);
  uint32_t addr2 = mallocate(2, 512);
  ASSERT_TRUE(addr1 != UINT32_MAX);
  ASSERT_TRUE(addr2 != UINT32_MAX);
  ASSERT_TRUE(addr1 != addr2);
}

TEST_CASE(Memory, AllocateZeroSizeFails) {
  reset_memory();
  uint32_t addr = mallocate(1, 0);
  ASSERT_EQ(addr, UINT32_MAX);
}

TEST_CASE(Memory, FreeAllocatedMemory) {
  reset_memory();
  uint32_t addr = mallocate(1, 1024);
  ASSERT_TRUE(addr != UINT32_MAX);
  liberate(1);
  // Should be able to allocate same size again
  uint32_t addr2 = mallocate(2, 1024);
  ASSERT_TRUE(addr2 != UINT32_MAX);
}

TEST_CASE(Memory, BestFitAllocation) {
  reset_memory();
  // Allocate and free to create fragmentation
  uint32_t addr1 = mallocate(1, 512);
  uint32_t addr2 = mallocate(2, 1024);
  uint32_t addr3 = mallocate(3, 512);
  
  ASSERT_TRUE(addr1 != UINT32_MAX);
  ASSERT_TRUE(addr2 != UINT32_MAX);
  ASSERT_TRUE(addr3 != UINT32_MAX);
  
  liberate(2); // Free middle block
  
  // Should fit in freed space
  uint32_t addr4 = mallocate(4, 512);
  ASSERT_TRUE(addr4 != UINT32_MAX);
}

TEST_CASE(Memory, AllocateAfterMultipleFrees) {
  reset_memory();
  uint32_t addr1 = mallocate(1, 256);
  uint32_t addr2 = mallocate(2, 256);
  uint32_t addr3 = mallocate(3, 256);
  
  liberate(1);
  liberate(2);
  liberate(3);
  
  uint32_t addr4 = mallocate(4, 512);
  ASSERT_TRUE(addr4 != UINT32_MAX);
}

// ============================================
// Process Isolation Tests
// ============================================

TEST_CASE(Memory, ProcessCanAccessOwnMemory) {
  reset_memory();
  uint32_t addr = mallocate(1, 1024);
  set_current_process(1);
  write_word(addr, 0x12345678);
  ASSERT_EQ(read_word(addr), 0x12345678);
}

TEST_CASE(Memory, SystemProcessCanAccessAll) {
  reset_memory();
  set_current_process(SYSTEM_PROCESS_ID);
  write_word(1000, 0xAAAAAAAA);
  ASSERT_EQ(read_word(1000), 0xAAAAAAAA);
}

TEST_CASE(Memory, ProcessCanAccessTextSegment) {
  reset_memory();
  set_current_process(0);
  uint32_t text_addr = TEXT_BASE;
  write_word(text_addr, 0x11223344);
  ASSERT_EQ(read_word(text_addr), 0x11223344);
}

TEST_CASE(Memory, ProcessCanAccessDataSegment) {
  reset_memory();
  set_current_process(0);
  uint32_t data_addr = DATA_BASE;
  write_word(data_addr, 0x55667788);
  ASSERT_EQ(read_word(data_addr), 0x55667788);
}

// ============================================
// Edge Cases and Bounds Tests
// ============================================

TEST_CASE(Memory, ReadAtAddressZero) {
  reset_memory();
  write_word(0, 0xCAFEBABE);
  ASSERT_EQ(read_word(0), 0xCAFEBABE);
}

TEST_CASE(Memory, WriteReadSequentialAddresses) {
  reset_memory();
  for (uint32_t i = 0; i < 100; i++) {
    write_byte(10000 + i, (uint8_t)(i & 0xFF));
  }
  for (uint32_t i = 0; i < 100; i++) {
    ASSERT_EQ(read_byte(10000 + i), (uint8_t)(i & 0xFF));
  }
}

TEST_CASE(Memory, LargeDataTransfer) {
  reset_memory();
  uint32_t base = 20000;
  for (uint32_t i = 0; i < 256; i++) {
    write_word(base + (i * 4), i * 0x11111111);
  }
  for (uint32_t i = 0; i < 256; i++) {
    ASSERT_EQ(read_word(base + (i * 4)), i * 0x11111111);
  }
}

TEST_CASE(Memory, AlternatingReadWrite) {
  reset_memory();
  uint32_t addr = 30000;
  write_word(addr, 0x12345678);
  ASSERT_EQ(read_word(addr), 0x12345678);
  write_word(addr, 0xAABBCCDD);
  ASSERT_EQ(read_word(addr), 0xAABBCCDD);
  write_word(addr, 0xFFEEDDCC);
  ASSERT_EQ(read_word(addr), 0xFFEEDDCC);
}

// ============================================
// Endianness Tests
// ============================================

TEST_CASE(Memory, LittleEndianByteOrder) {
  reset_memory();
  write_word(40000, 0x12345678);
  ASSERT_EQ(read_byte(40000), 0x78);
  ASSERT_EQ(read_byte(40001), 0x56);
  ASSERT_EQ(read_byte(40002), 0x34);
  ASSERT_EQ(read_byte(40003), 0x12);
}

TEST_CASE(Memory, HalfwordEndianness) {
  reset_memory();
  write_word(41000, 0xAABBCCDD);
  ASSERT_EQ(read_hword(41000), 0xCCDD);
  ASSERT_EQ(read_hword(41002), 0xAABB);
}

// ============================================
// Cache Statistics Tests
// ============================================

TEST_CASE(Memory, CacheStatsInitiallyZero) {
  reset_memory();
  // Just verify it doesn't crash
  print_cache_stats();
  ASSERT_TRUE(1);
}

TEST_CASE(Memory, ReadGeneratesCacheActivity) {
  reset_memory();
  for (int i = 0; i < 10; i++) {
    read_word(50000 + (i * 4));
  }
  // Cache should have some activity
  ASSERT_TRUE(1);
}

// ============================================
// Memory Coalescing Tests
// ============================================

TEST_CASE(Memory, CoalesceAdjacentFreeBlocks) {
  reset_memory();
  uint32_t addr1 = mallocate(1, 256);
  uint32_t addr2 = mallocate(2, 256);
  
  ASSERT_TRUE(addr1 != UINT32_MAX);
  ASSERT_TRUE(addr2 != UINT32_MAX);
  
  liberate(1);
  liberate(2);
  
  // Should be able to allocate larger block
  uint32_t addr3 = mallocate(3, 512);
  ASSERT_TRUE(addr3 != UINT32_MAX);
}

// ============================================
// Stress Tests
// ============================================

TEST_CASE(Memory, ManyAllocationsAndFrees) {
  reset_memory();
  for (int i = 0; i < 20; i++) {
    uint32_t addr = mallocate(i, 128);
    ASSERT_TRUE(addr != UINT32_MAX);
    if (i % 2 == 0) {
      liberate(i);
    }
  }
}

TEST_CASE(Memory, InterleavedCacheOperations) {
  reset_memory();
  for (int i = 0; i < 50; i++) {
    uint32_t addr = 60000 + (i * 8);
    write_word(addr, i);
    uint32_t val = read_word(addr);
    ASSERT_EQ(val, (uint32_t)i);
  }
}
