#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Memory layout constants
#define TEXT_BASE 0x00400000 // Location in memory to write instructions to
#define DATA_BASE 0x10010000 // Location in memory to write data to
#define STACK_TOP 0x7ffffffc // Top of stack, but probably not used for our assembler
#define GLOBAL_PTR 0x10008000       // Initialize $gp to middle of .text
#define MAX_PROCESS_SIZE 0x00100000 // 1MB per process
#define SYSTEM_PROCESS_ID -100

/*
 * The memory sytem to be put in place
 * could also be a boolean but this is easier to extend
 */
typedef enum {
  CACHE_WRITE_THROUGH,
  CACHE_WRITE_BACK
} CachePolicy;

// Initialize the memory and storage for the system
void init_memory(const CachePolicy policy);

// Free the memory allocated for the system
void free_memory(void);

/*
 * Return the byte at the given memory adress
 *
 * Parameters:
 *  addr: Memory adress of the byte to read
 */
uint8_t read_byte(uint32_t addr);

/*
 * Return the halfword (2 bytes) at the given memory adress
 *
 * Parameters:
 *  addr: Memory adress of the first byte to read
 */
uint16_t read_hword(uint32_t addr);

/*
 * Return the word (4 bytes) at the given memory adress
 *
 * Parameters:
 *  addr: Memory adress of the first byte to read
 */
uint32_t read_word(uint32_t addr);

/*
 * Write the given byte to the given memory adress
 *
 * Parameters:
 *  addr: Memory adress to begin writing to
 *  data: Data to write to the given adress
 */
void write_byte(uint32_t addr, uint8_t data);

/*
 * Write the given halfword (2 bytes) to the given memory adress
 *
 * Parameters:
 *  addr: Memory adress to begin writing to
 *  data: Data to write to the given adress
 */
void write_hword(uint32_t addr, uint16_t data);

/*
 * Write the given word (4 bytes) to the given memory adress
 *
 * Parameters:
 *  addr: Memory adress to begin writing to
 *  data: Data to write to the given adress
 */
void write_word(uint32_t addr, uint32_t data);

/*
 * Set the currently executing process (for access control)
 */
void set_current_process(int pid);

/*
 * Control whether liberate actually frees memory blocks.
 * Useful for comparison runs where the same allocations are
 * reused across multiple scheduler executions.
 */
void set_memory_freeze(bool freeze);

/*
 * Allocate memory for the given process
 *
 * Parameters:
 *  pid: Process id of the process requestin memory
 *  size: Size in number of bytes being requested
 *
 * Returns:
 *  Memory adress of the first allocated byte
 */
uint32_t mallocate(int pid, size_t size);

/*
 * Write the given halfword to the given memory adress
 *
 * Parameters:
 *  pid: Process id of the process to be freed
 */
void liberate(int pid);

// getters for memory stats
unsigned long get_L1_hits(void);
unsigned long get_L1_misses(void);
unsigned long get_L2_hits(void);
unsigned long get_L2_misses(void);
unsigned long get_write_backs(void);


// print the number of cache hits & misses
void print_cache_stats(void);
#endif
