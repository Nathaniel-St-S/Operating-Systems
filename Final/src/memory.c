// TODO add write no check
// TODO add security measures to stop execution check access fails
// TODO add checks for proper cache policy
// TODO addd write back cache policy
#include "../include/memory.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define L1CACHE_SIZE 32 * 1024
#define L2CACHE_SIZE 128 * 1024
#define CACHE_LINE_SIZE 64
#define RAM_SIZE 128 * 1024 * 1024
#define SSD_SIZE 256 * 1024 * 1024
#define HDD_SIZE 512 * 1024 * 1024
#define MAX_MEM_BLOCKS 500
#define MEMBLOCK(id) (MEMORY_TABLE.blocks[id])

#define EMPTY_ADDR -1
#define NO_PID -1
#define NO_VAL ((uint8_t)-1)

/* ---------------------------------------------------------------------------------------------------- */
/* ========================================= INTERNAL STRUCTS ========================================= */

/*
 * A cache line stores data in an array
 * to allow for multiple sizes of data
 * (byte, hword, word) wothout overwriting data
 * The tag represents the the adress data would be at
 */
typedef struct {
  uint32_t tag;
  bool is_valid;
  uint8_t data[CACHE_LINE_SIZE];
} CacheLine;

/*
 * Cache data structure, storing
 * an array of Entry as well as
 * a pointer to the next open
 * cache space. Also keeps track
 * of the number of items in cache
 * and the total size of the cache
 */
typedef struct {
  CacheLine *lines;
  size_t front;
  size_t count;
  size_t line_count;
} Cache;

/*
 * Memory block data structure that tracks
 * the appropriate starting and ending adresses
 * for the memory associated with a specific process
 * will also have a boolean 'isfree' for keeping track
 * of wether the memory is safe to be free'd
 */
typedef struct {
  int pid;
  uint32_t start_addr;
  uint32_t end_addr;
  bool is_free;
} MemoryBlock;

/*
 * Memory Table data structure to keep track of
 * the number of memory blocks and where they're
 * allocated
 */
typedef struct {
  MemoryBlock *blocks;
  size_t block_count;
  size_t capacity;
} MemoryTable;


/* ---------------------------------------------------------------------------------------------------- */
/* ========================================= FWD DECLARATIONS ========================================= */

// Initialize the memory and storage for the system
void init_memory(const CachePolicy policy);

// Free the memory allcoated for the system
void free_memory();

// return the byte at the given memory adress
uint8_t read_byte(uint32_t addr);

// return the halfword at the given memory adress
uint16_t read_hword(uint32_t addr);

// return the word at the iven memory adress
uint32_t read_word(uint32_t addr);

// write one byte of data to the given memory adress
void write_byte(uint32_t addr, uint8_t data);

// write a hword of data (2 bytes) to the given memory adress
void write_hword(uint32_t addr, uint16_t data);

// write one word of data (4 bytes) to the given memory adress
void write_word(uint32_t addr, uint32_t data);

// print the number of cache hits & misses
void print_cache_stats(void);

// allocate some memory for a process
uint32_t mallocate(int pid, size_t size);

// free up memory associated with a process
void liberate(int pid);

// the process with access rights
void set_current_process(int pid);

/* ---------------------------------------------------------------------------------------------------- */
/* ========================================= GLOBAL VARIABLES ========================================= */

// values for tracking cache stats
static unsigned long L1cache_hit = 0, L1cache_miss = 0;
static unsigned long L2cache_hit = 0, L2cache_miss = 0;

// L1 cache
static Cache L1;
// L2 cache
static Cache L2;
// RAM
static uint8_t *RAM = NULL;
// HDD
static uint8_t *HDD = NULL;
// SSD
static uint8_t *SSD = NULL;
// Memory Table
static MemoryTable MEMORY_TABLE = {0};
// Current process with memory acess rights
static int current_process_id = -1;

static CachePolicy cache_policy_type = CACHE_WRITE_THROUGH;

/* ---------------------------------------------------------------------------------------------------- */
/* ========================================== UTILITY FUNCTS ========================================== */

static inline uint32_t line_base(const uint32_t addr) {
  return addr & ~(CACHE_LINE_SIZE - 1u);
}

static bool in_bounds(const uint32_t base, const size_t size) {
  if (base > RAM_SIZE)
    return false;
  if (size > RAM_SIZE)
    return false;
  return size <= (RAM_SIZE - base);
}

// find the tag of the value if it exists in cache
static int find_line(const Cache *cache, const uint32_t base) {
  for (size_t i = 0; i < cache->line_count; i++) {
    // the address we want was found in cache
    // so return the index of that address
    if (cache->lines[i].is_valid && cache->lines[i].tag == base)
      return (int)i;
  }
  // address not found so return signifier
  return EMPTY_ADDR;
}

// Load a line from RAM into the cache
// Return the index used
static int load_line(Cache *cache, const uint32_t base) {
  if (base + CACHE_LINE_SIZE > RAM_SIZE) {
    fprintf(stderr, "cache [load line]: base out of RAM bounds (0x%08x)\n",
            base);
    exit(EXIT_FAILURE);
  }

  size_t index = 0;
  if (cache->count < cache->line_count) {
    index = (cache->front + cache->count) % cache->line_count;
    cache->count++;
  } else {
    index = cache->front;
    cache->front = (cache->front + 1) % cache->line_count;
  }

  cache->lines[index].tag = base;
  cache->lines[index].is_valid = true;
  memcpy(cache->lines[index].data, &RAM[base], CACHE_LINE_SIZE);
  return (int)index;
}

static bool check_access(uint32_t addr) {
  if (current_process_id == SYSTEM_PROCESS_ID) {
    return true; // System/kernel mode - allow all access
  }
  
  MemoryBlock *b = {0};
  bool valid_id = false;
  // Check if the process id is a valid id
  for (size_t i = 0; i < MEMORY_TABLE.block_count; i++){
    b = &MEMBLOCK(i);
    if (b->pid == current_process_id){
      valid_id = true;
      break;
    }
  }

  if (!valid_id){
    fprintf(stderr, "Pocess read/write access: Invalid process id\n");
    return false;
  }

  if (!b->is_free && addr >= b->start_addr && addr <= b->end_addr) return true;

  // Also check if address is in process's TEXT/DATA segments
  // Process 0: TEXT_BASE (0x00400000) to TEXT_BASE + 0x00100000
  // Process N: TEXT_BASE + (N * 0x00100000)
  // 1MB per process
  uint32_t process_text_start =
      TEXT_BASE + (current_process_id * MAX_PROCESS_SIZE);
  uint32_t process_text_end = process_text_start + MAX_PROCESS_SIZE;
  if (addr >= process_text_start && addr < process_text_end) {
    return true;
  }

  uint32_t process_data_start =
      DATA_BASE + (current_process_id * MAX_PROCESS_SIZE);
  uint32_t process_data_end = process_data_start + MAX_PROCESS_SIZE;
  if (addr >= process_data_start && addr < process_data_end) {
    return true;
  }

  return false;
}

static uint8_t read_byte_no_check(uint32_t addr){

  uint32_t base = line_base(addr);
  int idx = EMPTY_ADDR;

  // L1 Cache
  idx = find_line(&L1, base);
  if (idx != EMPTY_ADDR) {
    L1cache_hit++;
    return L1.lines[idx].data[addr - base];
  }
  L1cache_miss++;

  // L2 cache
  idx = find_line(&L2, base);
  if (idx != EMPTY_ADDR) {
    L2cache_hit++;
    // Copy to L1
    int l1_idx;
    if (L1.count < L1.line_count) {
      l1_idx = (L1.front + L1.count) % L1.line_count;
      L1.count++;
    } else {
      l1_idx = L1.front;
      L1.front = (L1.front + 1) % L1.line_count;
    }

    L1.lines[l1_idx].tag = base;
    L1.lines[l1_idx].is_valid = true;
    memcpy(L1.lines[l1_idx].data, L2.lines[idx].data,
        CACHE_LINE_SIZE); // Copy from L2!

    return L1.lines[l1_idx].data[addr - base];
  }
  L2cache_miss++;

  // Complete miss
  load_line(&L2, base);
  int l1_idx = load_line(&L1, base);
  return L1.lines[l1_idx].data[addr - base];
}

static void write_through_no_check (uint32_t addr, uint8_t value){
  
  RAM[addr] = value;

  uint32_t base = line_base(addr);
  int idx;

  // Update L1 if present
  idx = find_line(&L1, base);
  if (idx >= 0) {
    L1.lines[idx].data[addr - base] = value;
  }

  // Update L2 if present
  idx = find_line(&L2, base);
  if (idx >= 0) {
    L2.lines[idx].data[addr - base] = value;
  }
}

static void write_back_no_check (uint32_t addr, uint8_t);
/* ---------------------------------------------------------------------------------------------------- */
/* =========================================== INITIALIZERS =========================================== */

// Initialize the ram to the given size
static void init_ram(const size_t size) {
  RAM = calloc(size, 1);
  if (!RAM) {
    perror("calloc RAM");
    exit(EXIT_FAILURE);
  }
}

// Initialize the ssd to the given size
static void init_ssd(const size_t size) {
  SSD = calloc(size, 1);
  if (!SSD) {
    perror("calloc SSD");
    exit(EXIT_FAILURE);
  }
}

// Initialize the ssd to the given size
static void init_hdd(const size_t size) {
  HDD = calloc(size, 1);
  if (!HDD) {
    perror("calloc HDD");
    exit(EXIT_FAILURE);
  }
}

// Initialize the cache to the given size
static void init_cache(Cache *cache, const size_t size) {
  if ((size % CACHE_LINE_SIZE) != 0) {
    fprintf(stderr, "cache size must be multiple of LINE_SIZE\n");
    exit(EXIT_FAILURE);
  }
  cache->line_count = size / CACHE_LINE_SIZE;
  cache->front = 0;
  cache->count = 0;
  cache->lines = calloc(cache->line_count, sizeof(CacheLine));
  if (!cache->lines) {
    perror("calloc cache lines");
    exit(EXIT_FAILURE);
  }

  printf("Initialized cache at -> '%p' <- with | %lu | bytes, and | %lu | "
         "lines [Line Size = %d]\n",
         (void *)cache, size, cache->line_count, CACHE_LINE_SIZE);
}

// Initialize the memory table with one block of the entire memory space
static void init_memtab(const int num_blocks) {
  MEMORY_TABLE.capacity = num_blocks;
  MEMORY_TABLE.blocks = calloc(MEMORY_TABLE.capacity, sizeof(MemoryBlock));
  if (!MEMORY_TABLE.blocks) {
    perror("calloc meomory table blocks");
    exit(EXIT_FAILURE);
  }

  MEMBLOCK(0).pid = NO_PID;
  MEMBLOCK(0).start_addr = 0u;
  MEMBLOCK(0).end_addr = (uint32_t)(RAM_SIZE - 1u);
  MEMBLOCK(0).is_free = true;
  MEMORY_TABLE.block_count = 1;
}

void init_memory(const CachePolicy policy) {
  cache_policy_type = policy;
  init_ram(RAM_SIZE);
  init_ssd(SSD_SIZE);
  init_hdd(HDD_SIZE);
  init_cache(&L1, L1CACHE_SIZE);
  init_cache(&L2, L2CACHE_SIZE);
  init_memtab(MAX_MEM_BLOCKS);
}

/* ---------------------------------------------------------------------------------------------------- */
/* =========================================== DEALLOCATORS =========================================== */

static void free_cache(Cache *c) {
  free(c->lines);
  c->lines = NULL;
  c->line_count = c->front = c->count = 0;
}

void free_memory(void) {
  free(RAM);
  RAM = NULL;
  free(SSD);
  SSD = NULL;
  free(HDD);
  HDD = NULL;
  free_cache(&L1);
  free_cache(&L2);
  free(MEMORY_TABLE.blocks);
  MEMORY_TABLE.blocks = NULL;
  MEMORY_TABLE.block_count = MEMORY_TABLE.capacity = 0;
}

/* ---------------------------------------------------------------------------------------------------- */
/* ============================================ API FUNCTS ============================================ */

void set_current_process(const int pid) { current_process_id = pid; }

// Reads a single byte
// Uses the cache hierarchy
// Updates cache along the way
uint8_t read_byte(uint32_t addr) {
  if (!in_bounds(addr, 1)) {
    fprintf(stderr, "read [byte]: out of bounds addr=0x%08x\n", addr);
    return 0;
  }

  if (!check_access(addr)) {
    fprintf(stderr,
            "read [byte]: access violation - PID %d cannot access 0x%08x\n",
            current_process_id, addr);
    return 0;
  }

  return read_byte_no_check(addr);
}

// So called syntax sugar
uint16_t read_hword(uint32_t addr) {
  if (!in_bounds(addr, 2)) {
    fprintf(stderr, "read [hword]: out of bounds addr=0x%08x\n", addr);
    return 0;
  }

  if (!check_access(addr)) {
    fprintf(stderr,
            "read [hword]: access violation - PID %d cannot access 0x%08x\n",
            current_process_id, addr);
    return 0;
  }

  uint16_t b0 = (uint16_t)read_byte_no_check(addr);
  uint16_t b1 = (uint16_t)read_byte_no_check(addr + 1);
  return (uint16_t)(b0 | (b1 << 8));
}

uint32_t read_word(uint32_t addr) {
  if (!in_bounds(addr, 4)) {
    fprintf(stderr, "read [word]: out of bounds addr=0x%08x\n", addr);
    return 0;
  }

  if (!check_access(addr)) {
    fprintf(stderr,
        "read [word]: access violation - PID %d cannot access 0x%08x\n",
        current_process_id, addr);
    return 0;
  }

  uint32_t v = 0;
  v |= ((uint32_t)read_byte_no_check(addr + 0)) << 0;
  v |= ((uint32_t)read_byte_no_check(addr + 1)) << 8;
  v |= ((uint32_t)read_byte_no_check(addr + 2)) << 16;
  v |= ((uint32_t)read_byte_no_check(addr + 3)) << 24;
  return v;
}

void write_byte(uint32_t addr, uint8_t value) {
  if (!in_bounds(addr, 1)) {
    fprintf(stderr, "write [byte]: out of bounds addr=0x%08x\n", addr);
    return;
  }

  if (!check_access(addr)) {
    fprintf(stderr,
            "write [byte]: access violation - PID %d cannot write to 0x%08x\n",
            current_process_id, addr);
    return;
  }

  (cache_policy_type == CACHE_WRITE_THROUGH) ? write_through_no_check(addr, value) : write_back_no_check(addr, value);
}

void write_hword(uint32_t addr, uint16_t data) {
  if (!in_bounds(addr, 2)) {
    fprintf(stderr, "write [hword]: out of bounds addr=0x%08x\n", addr);
    return;
  }

  if (!check_access(addr)) {
    fprintf(stderr,
        "write [hword]: access violation - PID %d cannot write to 0x%08x\n",
        current_process_id, addr);
    return;
  }

  if (cache_policy_type == CACHE_WRITE_THROUGH){

    write_through_no_check(addr, (uint8_t)(data & 0xFF));
    write_through_no_check(addr + 1, (uint8_t)((data >> 8) & 0xFF));
  }
  else {

    write_back_no_check(addr, (uint8_t)(data & 0xFF));
    write_back_no_check(addr + 1, (uint8_t)((data >> 8) & 0xFF));
  }
}

void write_word(uint32_t addr, uint32_t data) {
  if (!in_bounds(addr, 4)) {
    fprintf(stderr, "write [word]: out of bounds addr=0x%08x\n", addr);
    return;
  }
  
  if (!check_access(addr)) {
    fprintf(stderr,
            "write [word]: access violation - PID %d cannot write to 0x%08x\n",
            current_process_id, addr);
    return;
  }

  if (cache_policy_type == CACHE_WRITE_THROUGH){

    write_through_no_check(addr + 0, (uint8_t)(data & 0xFF));
    write_through_no_check(addr + 1, (uint8_t)((data >> 8) & 0xFF));
    write_through_no_check(addr + 2, (uint8_t)((data >> 16) & 0xFF));
    write_through_no_check(addr + 3, (uint8_t)((data >> 24) & 0xFF));
  }
  else {

    write_back_no_check(addr + 0, (uint8_t)(data & 0xFF));
    write_back_no_check(addr + 1, (uint8_t)((data >> 8) & 0xFF));
    write_back_no_check(addr + 2, (uint8_t)((data >> 16) & 0xFF));
    write_back_no_check(addr + 3, (uint8_t)((data >> 24) & 0xFF));
  }
}

// Allocate memory for a specific process
uint32_t mallocate(int pid, size_t size) {
  if (size > UINT32_MAX) {
    fprintf(stderr, "mallocate: size too large. [4GB limit]\n");
    return UINT32_MAX;
  }

  if (size == 0)
    return UINT32_MAX;

  size_t best_idx = SIZE_MAX;
  uint32_t best_size = UINT32_MAX;

  // Find the best fitting block
  for (size_t i = 0; i < MEMORY_TABLE.block_count; ++i) {
    MemoryBlock *b = &MEMBLOCK(i);
    if (!b->is_free)
      continue;
    uint32_t bsize = (b->end_addr - b->start_addr) + 1u;
    if (bsize >= size && bsize < best_size) {
      best_size = bsize;
      best_idx = i;
    }
  }

  // No appropriate index found
  // requested more space than available
  if (best_idx == SIZE_MAX) {
    fprintf(stderr,
            "mallocate: could not fulfill pid=%d size=%zu — not enough free "
            "space\n",
            pid, size);
    return UINT32_MAX;
  }

  MemoryBlock *slot = &MEMBLOCK(best_idx);
  uint32_t old_start = slot->start_addr;
  uint32_t old_end = slot->end_addr;
  uint32_t new_end = old_start + (uint32_t)size - 1u;

  // Give the block to the process
  slot->pid = pid;
  slot->is_free = false;
  slot->start_addr = old_start;
  slot->end_addr = new_end;

  // Create a new block behind the allocated block if there is space
  if (new_end < old_end) {
    // Make sure there is enough space for a new free block
    if (MEMORY_TABLE.block_count + 1 > MEMORY_TABLE.capacity) {
      fprintf(stderr, "mallocate: memtable capacity reached\n");
      /* rollback */
      slot->pid = -1;
      slot->is_free = true;
      slot->start_addr = old_start;
      slot->end_addr = old_end;
      return UINT32_MAX;
    }

    // Shift blocks right to make room
    for (size_t i = MEMORY_TABLE.block_count; i > best_idx + 1; i--) {
      MEMBLOCK(i) = MEMBLOCK(i - 1);
    }

    // Create a new free block of the remaining space
    MEMBLOCK(best_idx + 1).pid = -1;
    MEMBLOCK(best_idx + 1).is_free = true;
    MEMBLOCK(best_idx + 1).start_addr = new_end + 1u;
    MEMBLOCK(best_idx + 1).end_addr = old_end;
    MEMORY_TABLE.block_count++;
  }

  printf("mallocate: PID %d allocated %zu bytes [%u -> %u]\n", pid, size,
         slot->start_addr, slot->end_addr);
  return slot->start_addr;
}

// Free up the memory allocated by a specific process.
void liberate(int pid) {
  size_t idx = SIZE_MAX;
  for (size_t i = 0; i < MEMORY_TABLE.block_count; ++i) {
    if (!MEMBLOCK(i).is_free && MEMBLOCK(i).pid == pid) {
      idx = i;
      break;
    }
  }
  if (idx == SIZE_MAX) {
    fprintf(stderr, "liberate: pid %d not found\n", pid);
    return;
  }

  MEMBLOCK(idx).is_free = true;
  MEMBLOCK(idx).pid = -1;
  printf("liberate: freed pid %d [%u -> %u]\n", pid, MEMBLOCK(idx).start_addr,
         MEMBLOCK(idx).end_addr);

  // Merge block with the previous if it is free
  if (idx > 0 && MEMBLOCK(idx - 1).is_free) {
    MEMBLOCK(idx - 1).end_addr = MEMBLOCK(idx).end_addr;
    // Shift left to make room
    for (size_t i = idx; i + 1 < MEMORY_TABLE.block_count; i++) {
      MEMBLOCK(i) = MEMBLOCK(i + 1);
    }
    MEMORY_TABLE.block_count--;
    idx--;
  }

  // Merge block with the next block if it is free
  if (idx + 1 < MEMORY_TABLE.block_count && MEMBLOCK(idx + 1).is_free) {
    MEMBLOCK(idx).end_addr = MEMBLOCK(idx + 1).end_addr;
    for (size_t j = idx + 1; j + 1 < MEMORY_TABLE.block_count; ++j) {
      MEMBLOCK(j) = MEMBLOCK(j + 1);
    }
    MEMORY_TABLE.block_count--;
  }
}

// print the number of cache hits & misses
void print_cache_stats() {
  printf("\nCache statistics:\n");
  printf("L1 hits:   %lu\n", L1cache_hit);
  printf("L1 misses: %lu\n", L1cache_miss);
  printf("L2 hits:   %lu\n", L2cache_hit);
  printf("L2 misses: %lu\n", L2cache_miss);
}
