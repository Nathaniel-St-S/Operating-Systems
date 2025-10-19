#ifndef MEMORY_H
#define MEMORY_H
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#define L1CACHE_SIZE 5
#define L2CACHE_SIZE 20
#define RAM_SIZE 500
#define SSD_SIZE 250
#define HDD_SIZE 1000
#define MAX_MEM_BLOCKS 500

#define EMPTY_ADDR -1
#define NO_PID -1
#define NO_VAL ((word) - 1)

/*
Individual entries in the cache
stores data as a key value pair
of memory adress and associated value
*/
typedef struct {
  word val;
  mem_addr addr;
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
  mem_addr start_addr;
  mem_addr end_addr;
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
extern word *RAM;

/*
Table to keep track of all the processes
and their associated memory blocks
*/
//extern MemoryBlock *MEMORY_TABLE;

// HDD,
extern word *HDD;

// SSD
extern word *SSD;

// Initialize the cache to the given size
void init_cache(Cache *cache, int size);

// initialize the ram to the given size
void init_ram(int size);

// initializes the SSD to the given size
void init_SSD(const int size);

// initializes the HDD to the given size
void init_HDD(const int size);

// return the value at the given memory adress
word read_mem(mem_addr addr);

// write the given value to the given memory adress.
void write_mem(mem_addr addr, const word val);

// print the number of cache hits & misses
void print_cache_stats(void);

// allocate some memory for a process
mem_addr mallocate(int pid, int size);

// free up memory associated with a process
void liberate(int pid);
#endif
