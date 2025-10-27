#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/interrupts.h"
#include "../include/dma.h"
#include "../include/processes.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// Thread synchronization flags
volatile int process1_done = 0;
volatile int process2_done = 0;
volatile int process3_done = 0;

// ============ PROCESS 1: Print "Hello, Professor" using CPU Interrupts ============
void* process1_hello_professor(void* arg) {
    printf("\n[Process 1 Started] - Printing 'Hello, Professor' using CPU interrupts\n");
    
    // Allocate memory for this process
    dword proc1_mem = mallocate(1, 100);
    
    // Store the string "Hello, Professor!\n" in memory (one char per dword)
    const char* message = "Hello, Professor!\n";
    int msg_len = 0;
    for (int i = 0; message[i] != '\0'; i++) {
        write_mem(proc1_mem + i, (dword)message[i]);
        msg_len++;
    }
    write_mem(proc1_mem + msg_len, 0); // Null terminator
    
    printf("[Process 1] Loaded message into memory at address 0x%X\n", proc1_mem);
    
    // Print the message 5 times using CPU interrupts
    for (int i = 0; i < 5; i++) {
        printf("[Process 1] Iteration %d/5: ", i + 1);
        
        // Set BX register to point to our string in memory
        THE_CPU.registers[BX] = proc1_mem;
        
        // Create and execute INT_PUTS interrupt instruction
        dword puts_instruction = (INTR << 24) | INT_PUTS;
        execute_instruction(INTR, puts_instruction);
        
        usleep(500000); // 0.5 seconds delay
    }
    
    // Store a completion marker in memory
    write_mem(proc1_mem + 50, 0xC0FFEE);
    printf("[Process 1] Stored completion marker at address 0x%X\n", proc1_mem + 50);
    
    // Free memory
    liberate(1);
    
    process1_done = 1;
    printf("[Process 1 Completed]\n\n");
    return NULL;
}

// ============ PROCESS 2: Complex Arithmetic Operations ============
void* process2_arithmetic(void* arg) {
    printf("\n[Process 2 Started] - Performing arithmetic operations\n");
    printf("[Process 2] Goal: Add to ACC until > 1,000,000, divide by 100, repeat 10 times, then multiply by 2 and store\n\n");
    
    // Allocate memory for this process
    dword proc2_mem = mallocate(2, 100);
    
    // Create a local CPU state for this process
    Cpu local_cpu;
    local_cpu.registers[ACC] = 0;
    local_cpu.registers[AX] = 1000000; // Target value
    local_cpu.registers[BX] = 100;     // Divisor
    local_cpu.registers[CX] = 0;       // Iteration counter
    
    printf("[Process 2] Starting with ACC = %u\n", local_cpu.registers[ACC]);
    
    // Repeat 10 times
    for (int iteration = 0; iteration < 10; iteration++) {
        printf("[Process 2] === Iteration %d/10 ===\n", iteration + 1);
        
        // Add to ACC until it's greater than 1,000,000
        int add_count = 0;
        while (local_cpu.registers[ACC] <= local_cpu.registers[AX]) {
            local_cpu.registers[ACC] += 12345; // Add increments
            add_count++;
        }
        
        printf("[Process 2] Added %d times, ACC = %u\n", add_count, local_cpu.registers[ACC]);
        
        // Divide by 100
        local_cpu.registers[ACC] /= local_cpu.registers[BX];
        printf("[Process 2] After division by 100: ACC = %u\n", local_cpu.registers[ACC]);
        
        usleep(300000); // 0.3 seconds delay
    }
    
    // Multiply final result by 2
    local_cpu.registers[ACC] *= 2;
    printf("\n[Process 2] Final ACC value after multiplying by 2: %u\n", local_cpu.registers[ACC]);
    
    // Store the result in memory
    dword storage_addr = proc2_mem + 10;
    write_mem(storage_addr, local_cpu.registers[ACC]);
    printf("[Process 2] Stored final result %u at memory address 0x%X\n", 
           local_cpu.registers[ACC], storage_addr);
    
    // Verify the stored value
    dword verify = read_mem(storage_addr);
    printf("[Process 2] Verification: Read back value %u from memory\n", verify);
    
    // Free memory
    liberate(2);
    
    process2_done = 1;
    printf("[Process 2 Completed]\n\n");
    return NULL;
}

// ============ PROCESS 3: DMA Transfer from SSD/HDD ============
void* process3_dma_transfer(void* arg) {
    printf("\n[Process 3 Started] - Waiting for DMA transfer from SSD/HDD\n");
    
    // Allocate memory for this process
    dword proc3_mem = mallocate(3, 200);
    
    printf("[Process 3] Simulating data on SSD and HDD...\n");
    
    // Prepare data on SSD
    for (int i = 0; i < 10; i++) {
        SSD[i] = 0xAA00 + i;
    }
    printf("[Process 3] Prepared 10 words on SSD\n");
    
    // Prepare data on HDD
    for (int i = 0; i < 15; i++) {
        HDD[i] = 0xBB00 + i;
    }
    printf("[Process 3] Prepared 15 words on HDD\n");
    
    // Simulate waiting for I/O
    printf("[Process 3] Waiting for I/O operations...\n");
    usleep(1000000); // 1 second delay
    
    // DMA Transfer from SSD to RAM
    printf("[Process 3] Initiating DMA transfer from SSD to RAM...\n");
    initiateDMA(SSD, &RAM[proc3_mem], 10);
    printf("[Process 3] DMA transfer from SSD complete\n");
    
    // Verify SSD transfer
    printf("[Process 3] Verifying SSD data in RAM:\n");
    for (int i = 0; i < 10; i++) {
        printf("  RAM[%d] = 0x%X (expected 0x%X)\n", 
               proc3_mem + i, RAM[proc3_mem + i], 0xAA00 + i);
    }
    
    usleep(500000); // 0.5 seconds delay
    
    // DMA Transfer from HDD to RAM
    printf("[Process 3] Initiating DMA transfer from HDD to RAM...\n");
    initiateDMA(HDD, &RAM[proc3_mem + 20], 15);
    printf("[Process 3] DMA transfer from HDD complete\n");
    
    // Verify HDD transfer
    printf("[Process 3] Verifying HDD data in RAM:\n");
    for (int i = 0; i < 15; i++) {
        printf("  RAM[%d] = 0x%X (expected 0x%X)\n", 
               proc3_mem + 20 + i, RAM[proc3_mem + 20 + i], 0xBB00 + i);
    }
    
    // Process the transferred data
    dword sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += RAM[proc3_mem + i];
    }
    printf("[Process 3] Sum of SSD data: 0x%X\n", sum);
    
    sum = 0;
    for (int i = 0; i < 15; i++) {
        sum += RAM[proc3_mem + 20 + i];
    }
    printf("[Process 3] Sum of HDD data: 0x%X\n", sum);
    
    // Free memory
    liberate(3);
    
    process3_done = 1;
    printf("[Process 3 Completed]\n\n");
    return NULL;
}

// ============ INTERRUPT GENERATORS ============
void* timer_interrupt_thread(void* arg) {
    int count = 0;
    while (!process1_done || !process2_done || !process3_done) {
        sleep(2);
        add_interrupt(SAY_HI, 5);
        count++;
        if (count >= 3) break; // Limit interrupts
    }
    return NULL;
}

void* io_interrupt_thread(void* arg) {
    int count = 0;
    while (!process1_done || !process2_done || !process3_done) {
        sleep(3);
        add_interrupt(SAY_GOODBYE, 3);
        count++;
        if (count >= 2) break; // Limit interrupts
    }
    return NULL;
}

// ============ DEMO CPU PROGRAM ============
void load_demo_program() {
    printf("\n[System] Loading demo CPU program into memory...\n");
    
    // Simple program that demonstrates the instruction set
    int addr = 0;
    
    // Initialize some values in memory
    write_mem(0x100, 42);
    write_mem(0x101, 10);
    
    // Program: Load, Add, Multiply, Store
    // LOAD AX, 0x100
    RAM[addr++] = (LOAD << 24) | (AX << 20) | 0x100;
    
    // ADD AX, AX, #8 (immediate)
    RAM[addr++] = (ADD << 24) | (AX << 20) | (AX << 16) | (1 << 12) | 8;
    
    // MUL BX, AX, #2 (immediate)
    RAM[addr++] = (MUL << 24) | (BX << 20) | (AX << 16) | (1 << 12) | 2;
    
    // STORE BX, 0x102
    RAM[addr++] = (STORE << 24) | (BX << 20) | 0x102;
    
    // LOAD CX, 0x101
    RAM[addr++] = (LOAD << 24) | (CX << 20) | 0x101;
    
    // SUB DX, BX, CX
    RAM[addr++] = (SUB << 24) | (DX << 20) | (BX << 16) | (CX);
    
    // STORE DX, 0x103
    RAM[addr++] = (STORE << 24) | (DX << 20) | 0x103;
    
    // HALT instruction - use INT_HALT interrupt
    RAM[addr++] = (INTR << 24) | INT_HALT;
    
    printf("[System] Demo program loaded (%d instructions)\n\n", addr);
}

// ============ MAIN ============
int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        Advanced Operating System Simulator Demo              ║\n");
    printf("║              Project 2 - Fall 2025                           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    // ========== INITIALIZATION ==========
    printf("\n[INITIALIZATION PHASE]\n");
    printf("═══════════════════════\n");
    
    init_cache(&L1, L1CACHE_SIZE);
    init_cache(&L2, L2CACHE_SIZE);
    init_ram(RAM_SIZE);
    init_HDD(HDD_SIZE);
    init_SSD(SSD_SIZE);
    init_interrupt_controller();
    init_cpu(&THE_CPU);
    init_processes();
    
    printf("\n✓ All systems initialized successfully\n");
    
    // ========== LOAD DEMO PROGRAM ==========
    load_demo_program();
    
    // ========== CREATE PROCESS THREADS ==========
    printf("\n[PROCESS EXECUTION PHASE]\n");
    printf("═══════════════════════════\n");
    printf("\nStarting 3 concurrent processes...\n");
    
    pthread_t proc1_thread, proc2_thread, proc3_thread;
    pthread_t timer_thread, io_thread;
    
    // Launch interrupt generators
    pthread_create(&timer_thread, NULL, timer_interrupt_thread, NULL);
    pthread_create(&io_thread, NULL, io_interrupt_thread, NULL);
    
    // Launch process threads
    pthread_create(&proc1_thread, NULL, process1_hello_professor, NULL);
    pthread_create(&proc2_thread, NULL, process2_arithmetic, NULL);
    pthread_create(&proc3_thread, NULL, process3_dma_transfer, NULL);
    
    // Wait for all processes to complete
    pthread_join(proc1_thread, NULL);
    pthread_join(proc2_thread, NULL);
    pthread_join(proc3_thread, NULL);
    
    printf("\n[CPU EXECUTION PHASE]\n");
    printf("═══════════════════════\n");
    printf("\nExecuting loaded CPU program...\n\n");
    
    // Run the CPU with the loaded program
    cpu_run(7);
    
    // ========== RESULTS AND STATISTICS ==========
    printf("\n[RESULTS AND STATISTICS]\n");
    printf("═══════════════════════════\n");
    
    printf("\nFinal Memory Contents:\n");
    printf("  Address 0x100: 0x%X (initial value)\n", read_mem(0x100));
    printf("  Address 0x101: 0x%X (initial value)\n", read_mem(0x101));
    printf("  Address 0x102: 0x%X (result of computation)\n", read_mem(0x102));
    printf("  Address 0x103: 0x%X (result of subtraction)\n", read_mem(0x103));
    
    print_cache_stats();
    
    printf("\nCPU Final State:\n");
    printf("  AX: 0x%X\n", THE_CPU.registers[AX]);
    printf("  BX: 0x%X\n", THE_CPU.registers[BX]);
    printf("  CX: 0x%X\n", THE_CPU.registers[CX]);
    printf("  DX: 0x%X\n", THE_CPU.registers[DX]);
    
    // ========== CLEANUP ==========
    printf("\n[CLEANUP PHASE]\n");
    printf("═══════════════\n");
    
    // Cancel interrupt threads
    pthread_cancel(timer_thread);
    pthread_cancel(io_thread);
    pthread_join(timer_thread, NULL);
    pthread_join(io_thread, NULL);
    
    // Free allocated resources
    free_interrupt_controller();
    free(L1.items);
    free(L2.items);
    free(RAM);
    free(HDD);
    free(SSD);
    
    printf("\n✓ All resources freed successfully\n");
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                  Demo Completed Successfully                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Press Enter to exit...");
    getchar();
    
    return 0;
}
