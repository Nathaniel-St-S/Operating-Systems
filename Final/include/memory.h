#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define L1CACHE_SIZE 5
#define L2CACHE_SIZE 20
#define RAM_SIZE 500
#define SSD_SIZE 250
#define HDD_SIZE 1000
#define MAX_MEM_BLOCKS 500

#define EMPTY_ADDR -1
#define NO_PID -1
#define NO_VAL ((uint32_t) - 1)

/*
Individual entries in the cache
stores data as a key value pair
of memory adress and associated value
*/
typedef struct {
  uint32_t val;
  uint32_t addr;
} Entry;

/*
Cache data structure, storing
an array of Entry as well as
a pointer to the next open
cache space. Also keeps track
of the number of items in cache
and the total size of the cache
*/
typedef struct {
  Entry *items;
  int front;
  int count;
  int size;
} Cache;

/*
Memory block data structure that tracks
the appropriate starting and ending adresses
for the memory associated with a specific process
will also have a boolean 'isfree' for keeping track
of wether the memory is safe to be free'd
*/
typedef struct {
  int pid;
  uint32_t start_addr;
  uint32_t end_addr;
  bool is_free;
} MemoryBlock;

/*
Memory Table data structure to keep track of
the number of memory blocks and where they're
allocated
*/
typedef struct
{
  MemoryBlock *blocks;
  int block_count;
} MemoryTable;

// Track the number of misses for stats
//extern int L1cache_hit, L1cache_miss;
//extern int L2cache_hit, L2cache_miss;

// Level 1 cache, small and quick
extern Cache L1;

// Level 2 cache, larger but slower
extern Cache L2;

// Ram, large amounts of storage but very slow
extern uint32_t *RAM;

/*
Table to keep track of all the processes
and their associated memory blocks
*/
//extern MemoryBlock *MEMORY_TABLE;

// HDD,
extern uint32_t *HDD;

// SSD
extern uint32_t *SSD;

// Initialize the cache to the given size
void init_cache(Cache *cache, int size);

// initialize the ram to the given size
void init_ram(int size);

// initializes the SSD to the given size
void init_SSD(const int size);

// initializes the HDD to the given size
void init_HDD(const int size);

// return the value at the given memory adress
uint32_t read_mem(uint32_t addr);

// write the given value to the given memory adress.
void write_mem(uint32_t addr, const uint32_t val);

// print the number of cache hits & misses
void print_cache_stats(void);

// allocate some memory for a process
uint32_t mallocate(int pid, int size);

// free up memory associated with a process
void liberate(int pid);
#endif
