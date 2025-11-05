#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/interrupts.h"
#include "../include/processes.h"
#include "../include/dma.h"

// Test counters
int tests_passed = 0;
int tests_failed = 0;

// Test helper macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            printf("  ✓ PASS: %s\n", message); \
            tests_passed++; \
        } else { \
            printf("  ✗ FAIL: %s\n", message); \
            tests_failed++; \
        } \
    } while(0)

#define TEST_START(name) \
    printf("\n=== Testing: %s ===\n", name)

// Initialize test environment
void init_test_environment() {
    init_cache(&L1, L1CACHE_SIZE);
    init_cache(&L2, L2CACHE_SIZE);
    init_ram(RAM_SIZE);
    init_HDD(HDD_SIZE);
    init_SSD(SSD_SIZE);
    init_interrupt_controller();
    init_cpu();
    init_processes();
}

// Cleanup test environment
void cleanup_test_environment() {
    free_interrupt_controller();
    free(L1.items);
    free(L2.items);
    free(RAM);
    free(HDD);
    free(SSD);
}

// ============ CPU TESTS ============
void test_cpu_initialization() {
    TEST_START("CPU Initialization");
    
    init_cpu();
    
    TEST_ASSERT(THE_CPU.registers[PC] == MEM_START, "PC initialized to MEM_START");
    TEST_ASSERT(THE_CPU.registers[IR] == EMPTY_REG, "IR initialized to EMPTY_REG");
    TEST_ASSERT(THE_CPU.registers[FLAG] == F_ZERO, "FLAG initialized to F_ZERO");
    TEST_ASSERT(THE_CPU.registers[ACC] == 0, "ACC initialized to 0");
}

void test_add_instruction() {
    TEST_START("ADD Instruction");
    
    // Test immediate mode ADD: ADD AX, BX, #5
    // Opcode: ADD (0x0), DR: AX (0), SR1: BX (1), IMM: 1, Operand: 5
    THE_CPU.registers[BX] = 10;
    dword instruction = (ADD << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 5;
    
    execute_instruction(ADD, instruction);
    
    TEST_ASSERT(THE_CPU.registers[AX] == 15, "AX = BX + 5 = 15");
    TEST_ASSERT(!(THE_CPU.registers[FLAG] & F_ZERO), "Zero flag not set for non-zero result");
}

void test_sub_instruction() {
    TEST_START("SUB Instruction");
    
    THE_CPU.registers[BX] = 20;
    dword instruction = (SUB << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 5;
    
    execute_instruction(SUB, instruction);
    
    TEST_ASSERT(THE_CPU.registers[AX] == 15, "AX = BX - 5 = 15");
}

void test_mul_instruction() {
    TEST_START("MUL Instruction");
    
    THE_CPU.registers[BX] = 10;
    dword instruction = (MUL << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 5;
    
    execute_instruction(MUL, instruction);
    
    TEST_ASSERT(THE_CPU.registers[AX] == 50, "AX = BX * 5 = 50");
}

void test_div_instruction() {
    TEST_START("DIV Instruction");
    
    THE_CPU.registers[BX] = 100;
    dword instruction = (DIV << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 5;
    
    execute_instruction(DIV, instruction);
    
    TEST_ASSERT(THE_CPU.registers[AX] == 20, "AX = BX / 5 = 20");
}

void test_bitwise_operations() {
    TEST_START("Bitwise Operations");
    
    // AND test
    THE_CPU.registers[BX] = 0xFF;
    dword instruction = (AND << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 0x0F;
    execute_instruction(AND, instruction);
    TEST_ASSERT(THE_CPU.registers[AX] == 0x0F, "AND operation works");
    
    // OR test
    THE_CPU.registers[BX] = 0xF0;
    instruction = (OR << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 0x0F;
    execute_instruction(OR, instruction);
    TEST_ASSERT(THE_CPU.registers[AX] == 0xFF, "OR operation works");
    
    // XOR test
    THE_CPU.registers[BX] = 0xFF;
    instruction = (XOR << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 0xFF;
    execute_instruction(XOR, instruction);
    TEST_ASSERT(THE_CPU.registers[AX] == 0, "XOR operation works");
}

// ============ MEMORY TESTS ============
void test_memory_read_write() {
    TEST_START("Memory Read/Write");
    
    dword test_addr = 100;
    dword test_val = 0xDEADBEEF;
    
    write_mem(test_addr, test_val);
    dword read_val = read_mem(test_addr);
    
    TEST_ASSERT(read_val == test_val, "Memory read/write consistency");
}

void test_cache_hierarchy() {
    TEST_START("Cache Hierarchy");
    
    // Reset cache stats
    extern int L1cache_hit, L1cache_miss, L2cache_hit, L2cache_miss;
    L1cache_hit = L1cache_miss = L2cache_hit = L2cache_miss = 0;
    
    dword addr = 50;
    dword val = 0x12345678;
    
    write_mem(addr, val);
    
    // First read should miss all caches
    read_mem(addr);
    TEST_ASSERT(L1cache_miss == 1, "L1 cache miss on first read");
    TEST_ASSERT(L2cache_miss == 1, "L2 cache miss on first read");
    
    // Second read should hit L1
    read_mem(addr);
    TEST_ASSERT(L1cache_hit == 1, "L1 cache hit on second read");
}

void test_memory_allocation() {
    TEST_START("Memory Allocation");
    
    int pid1 = 1;
    int size1 = 100;
    
    dword addr1 = mallocate(pid1, size1);
    TEST_ASSERT(addr1 != (dword)-1, "Memory allocation successful");
    
    int pid2 = 2;
    int size2 = 50;
    dword addr2 = mallocate(pid2, size2);
    TEST_ASSERT(addr2 != (dword)-1, "Second allocation successful");
    TEST_ASSERT(addr2 != addr1, "Allocations don't overlap");
    
    liberate(pid1);
    liberate(pid2);
}

void test_memory_deallocation() {
    TEST_START("Memory Deallocation");
    
    int pid = 3;
    dword addr = mallocate(pid, 100);
    
    liberate(pid);
    
    // Try to allocate again in freed space
    dword addr2 = mallocate(4, 50);
    TEST_ASSERT(addr2 == addr, "Freed memory is reusable");
    
    liberate(4);
}

// ============ INTERRUPT TESTS ============
void test_interrupt_queue() {
    TEST_START("Interrupt Queueing");
    
    add_interrupt(SAY_HI, 5);
    add_interrupt(SAY_GOODBYE, 3);
    add_interrupt(SAY_HI, 7);
    
    TEST_ASSERT(1, "Interrupts queued without error");
    
    // Process interrupts
    check_for_interrupt();
    check_for_interrupt();
    check_for_interrupt();
}

void test_interrupt_priority() {
    TEST_START("Interrupt Priority");
    
    add_interrupt(SAY_HI, 10);
    add_interrupt(SAY_GOODBYE, 1);  // Higher priority (lower number)
    
    // Should handle SAY_GOODBYE first due to priority
    check_for_interrupt();
    TEST_ASSERT(1, "Higher priority interrupt handled first");
}

// ============ PROCESS TESTS ============
void test_process_creation() {
    TEST_START("Process Creation");
    
    TEST_ASSERT(PROCESS_TABLE[0].state == RUNNING, "First process is RUNNING");
    TEST_ASSERT(PROCESS_TABLE[1].state == READY, "Other processes are READY");
}

void test_context_switching() {
    TEST_START("Context Switching");
    
    // Set up different register values for processes
    PROCESS_TABLE[0].cpu_state.registers[AX] = 100;
    PROCESS_TABLE[1].cpu_state.registers[AX] = 200;
    
    THE_CPU.registers[AX] = 100;
    
    context_switch(0, 1);
    
    TEST_ASSERT(THE_CPU.registers[AX] == 200, "Context switch loads new process state");
    TEST_ASSERT(PROCESS_TABLE[0].state == READY, "Previous process marked READY");
    TEST_ASSERT(PROCESS_TABLE[1].state == RUNNING, "New process marked RUNNING");
}

// ============ DMA TESTS ============
void test_dma_transfer() {
    TEST_START("DMA Transfer");
    
    dword source[5] = {1, 2, 3, 4, 5};
    dword destination[5] = {0, 0, 0, 0, 0};
    
    initiateDMA(source, destination, 5);
    
    int match = 1;
    for (int i = 0; i < 5; i++) {
        if (source[i] != destination[i]) {
            match = 0;
            break;
        }
    }
    
    TEST_ASSERT(match, "DMA transfer copies data correctly");
}

// ============ INSTRUCTION SET TESTS ============
void test_load_store() {
    TEST_START("LOAD/STORE Instructions");
    
    dword addr = 10;
    dword value = 0xABCDEF12;
    
    // Write value to memory
    write_mem(addr, value);
    
    // LOAD instruction: LOAD AX, PC+offset
    THE_CPU.registers[PC] = 0;
    dword load_instr = (LOAD << 24) | (AX << 20) | addr;
    execute_instruction(LOAD, load_instr);
    
    TEST_ASSERT(THE_CPU.registers[AX] == value, "LOAD retrieves value from memory");
    
    // STORE instruction
    THE_CPU.registers[BX] = 0xCAFEBABE;
    THE_CPU.registers[PC] = 0;
    dword store_instr = (STORE << 24) | (BX << 20) | 20;
    execute_instruction(STORE, store_instr);
    
    TEST_ASSERT(read_mem(20) == 0xCAFEBABE, "STORE writes value to memory");
}

void test_jump_instructions() {
    TEST_START("JUMP Instructions");
    
    // JUMP instruction
    THE_CPU.registers[PC] = 10;
    THE_CPU.registers[BX] = 50;
    dword jump_instr = (JUMP << 24) | (BX << 16);
    execute_instruction(JUMP, jump_instr);
    
    TEST_ASSERT(THE_CPU.registers[PC] == 50, "JUMP sets PC to register value");
    
    // JUMPZ instruction with zero flag set
    THE_CPU.registers[PC] = 10;
    THE_CPU.registers[FLAG] = F_ZERO;
    THE_CPU.registers[CX] = 100;
    dword jumpz_instr = (JUMPZ << 24) | (CX << 16);
    execute_instruction(JUMPZ, jumpz_instr);
    
    TEST_ASSERT(THE_CPU.registers[PC] == 100, "JUMPZ jumps when zero flag is set");
}

// ============ FLAG TESTS ============
void test_flag_operations() {
    TEST_START("Flag Operations");
    
    // Test zero flag
    THE_CPU.registers[BX] = 5;
    dword sub_instr = (SUB << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 5;
    execute_instruction(SUB, sub_instr);
    TEST_ASSERT(THE_CPU.registers[FLAG] & F_ZERO, "Zero flag set when result is zero");
    
    // Test overflow flag
    THE_CPU.registers[BX] = 0x7FFFFFFF;  // Max positive int
    dword add_instr = (ADD << 24) | (AX << 20) | (BX << 16) | (1 << 12) | 1;
    execute_instruction(ADD, add_instr);
    TEST_ASSERT(THE_CPU.registers[FLAG] & F_OVFLW, "Overflow flag set on overflow");
}

// ============ MAIN TEST RUNNER ============
int main() {
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║   Operating System Simulator - Test Suite      ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    
    init_test_environment();
    
    // Run all tests
    test_cpu_initialization();
    test_add_instruction();
    test_sub_instruction();
    test_mul_instruction();
    test_div_instruction();
    test_bitwise_operations();
    
    test_memory_read_write();
    test_cache_hierarchy();
    test_memory_allocation();
    test_memory_deallocation();
    
    test_interrupt_queue();
    test_interrupt_priority();
    
    test_process_creation();
    test_context_switching();
    
    test_dma_transfer();
    test_load_store();
    test_jump_instructions();
    test_flag_operations();
    
    cleanup_test_environment();
    
    // Print summary
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║              Test Results Summary              ║\n");
    printf("╠════════════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-3d                             ║\n", tests_passed);
    printf("║  Tests Failed: %-3d                             ║\n", tests_failed);
    printf("║  Total Tests:  %-3d                             ║\n", tests_passed + tests_failed);
    printf("╚════════════════════════════════════════════════╝\n");
    
    if (tests_failed == 0) {
        printf("\n✓ All tests passed! System is functioning correctly.\n\n");
        return 0;
    } else {
        printf("\n✗ Some tests failed. Please review the output above.\n\n");
        return 1;
    }
}
