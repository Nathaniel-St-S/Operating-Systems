#ifndef MEMORY_H
#define MEMORY_H
#include <stdio.h>
#include <stdlib.h>
#include "types.h"

#define L1CACHE_SIZE 5
#define L2CACHE_SIZE 20
#define RAM_SIZE 500

#define EMPTY_ADDR -1
#define NO_VAL ((word) - 1)

/*
Individual entries in the cache
stores data as a key value pair
of memory adress and associated value
*/
typedef struct
{
	word val;
	mem_addr addr;
}Entry;

/*
Cache data structure, storing
an array of Entry as well as
a pointer to the next open
cache space. Also keeps track
of the number of items in cache
and the total size of the cache
*/
typedef struct
{
	Entry* items;
	int front;
	int count;
	int size;
} Cache;

//Track the number of misses for stats
extern int L1cache_hit, L1cache_miss;
extern int L2cache_hit, L2cache_miss;

//Level 1 cache, small and quick
extern Cache L1;

//Level 2 cache, larger but slower
extern Cache L2;

//Ram, large amounts of storage but very slow
extern word* RAM;

//Initialize the cache to the given size
void init_cache(Cache* cache, int size);

//initialize the ram to the given size
void init_ram(int size);

 //return the value at the given memory adress
word read_mem(mem_addr addr);

//write the given value to the given memory adress.
void write_mem(mem_addr addr, const word val);

 //print the number of cache hits & misses
void print_cache_stats(void);

#endif
