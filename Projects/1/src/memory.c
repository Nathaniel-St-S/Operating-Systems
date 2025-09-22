#include "../include/memory.h"

//values for tracking cache stats
int L1cache_hit = 0, L1cache_miss = 0;
int L2cache_hit = 0, L2cache_miss = 0;

//L1 cache
Cache L1;
//L2 cache
Cache L2;
//RAM
word* RAM = NULL;
//HDD
word* HDD = NULL;
//SSD
word* SSD = NULL;


//Initialize the cache to the given size
void init_cache(Cache* cache, const int size)
{
	cache->items = (Entry*)malloc(sizeof(Entry) * size);
	cache->front = 0;
	cache->count = 0;
	cache->size = size;

	for(int i = 0; i < size; i++)
	{
		cache->items[i].addr = EMPTY_ADDR;
		cache->items[i].val = NO_VAL;
	}
	printf("Initialized cache at -> '%p' <- with size: %d\n",cache, size);
}

//initialize the ram to the given size
void init_ram(const int size)
{
	RAM = (word*)malloc(sizeof(word) * size);
	for(int i = 0; i < size; i++)
	{
		RAM[i] = NO_VAL;
	}
	printf("initialized ram with size: %d\n", size);
}

//initializes the SSD to the given size 
void init_SSD(const int size) {
	SSD = (word*)malloc(sizeof(word) * size);
	for(int i = 0; i < size; i++)
	{
		SSD[i] = NO_VAL;
	}
}

//initializes the HDD to the given size 
void init_HDD(const int size) {
	HDD = (word*)malloc(sizeof(word) * size);
	for(int i = 0; i < size; i++)
	{
		HDD[i] = NO_VAL;
	}
}

//find the address of the value if it exists in cache
int cache_search(Cache* cache, const mem_addr addr)
{
	for(int i = 0; i < cache->size; i++)
	{
		//the address we want was found in cache
		//so return the index of that adress
		if(cache->items[i].addr == addr)
		{
			return i;
		}
	}
	//address not found so return signifier
	return EMPTY_ADDR;
}

//Update the given cache in case of misses
void update_cache(Cache* cache, const mem_addr addr, const word val)
{
	//calculate where in the cache to store the value
	int index = (cache->front + cache->count) % cache->size;
	cache->items[index].addr = addr;
	cache->items[index].val = val;

	//update the size and count of the cache
	if(cache->count < cache->size)
	{
		//cache isn't full so we can just put the new
		//value in the next index in the cache
		cache->count++;
	}
	else
	{
		//cache is full, so loop around to put the new
		//value at the front
		cache->front = (cache->front + 1) % cache->size;
	}
}

//return the value at the given memory adress
word read_mem(const mem_addr addr)
{
	int index;
	index = cache_search(&L1, addr);
	if(index != EMPTY_ADDR)
	{
		//Cache hit at L1
		L1cache_hit++;
		return L1.items[index].val;
	}

	//cache miss at L1
	L1cache_miss++;

	index = cache_search(&L2, addr);
	if(index != EMPTY_ADDR)
	{
		//Cache hit at l2
		L2cache_hit++;
		word val = L2.items[index].val;
		//Update L1 cache to prevent future cache misses
		update_cache(&L1, addr, val);
		return val;
	}
	//Cache miss at L2
	L2cache_miss++;

	//Complete cache miss, so read RAM and update cache
	word val = RAM[addr];
	update_cache(&L1, addr, val);
	update_cache(&L2, addr, val);
	return val;
}

//write the given value to the given memory adress.
void write_mem(const mem_addr addr, const word val)
{
	RAM[addr] = val;

	int index;
	
	//Update L1 Cache
	index = cache_search(&L1, addr);
	if(index != EMPTY_ADDR)
	{
		L1.items[index].val = val;
	}

	//Update L2 Cache
	index = cache_search(&L2, addr);
	if(index != EMPTY_ADDR)
	{
		L2.items[index].val = val;
	}
}

//print the number of cache hits & misses
void print_cache_stats()
{
    printf("\nCache statistics:\n");
    printf("L1 hits:   %d\n", L1cache_hit);
    printf("L1 misses: %d\n", L1cache_miss);
    printf("L2 hits:   %d\n", L2cache_hit);
    printf("L2 misses: %d\n", L2cache_miss);
}
