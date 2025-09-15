#include <stdio.h>
#include <stdlib.h>
#include "../include/types.h"

#define L1CACHE_SIZE 64000
#define L2CACHE_SIZE 1000000
#define RAM_SIZE 5000000000
#define HEX_SIZE 5

//Return the value at the given memory adress
char read_mem(uword addr);

//Write given value to the given adress, 
void write_mem(uword addr, char* val);

//Update the given cache in case of misses
update_cache(char* cache, int cache_size, uword addr, char* val)

int main()
{
	//uword* l1_ptr = (uword*)malloc(sizeof(uword) * L1CACHE_SIZE);
	//uword* l2_ptr = (uword*)malloc(sizeof(uword) * L2CACHE_SIZE);
	//uword* ram_ptr = (uword*)malloc(sizeof(uword) * RAM_SIZE)
	char l1[L1CACHE_SIZE][HEX_SIZE];
	char l2[L2CACHE_SIZE][HEX_SIZE];
	char ram[RAM_SIZE][HEX_SIZE];	
}

char read_mem(uword addr)
{
	if(in_cache(l1, L1CACHE_SIZE, addr))
	{
		//Cache hit at l1
		L1cache_hit++;
		return L1[addr];
	}

	//cache miss at L1
	l1cache_miss++;

	if(in_cache(l2, L2CACHE_SIZE, addr))
	{
		//Cache hit at l2
		L2cache_hit++;
		//Update L1 cache to prevent future cache misses
		update_cache(L1, L1CACHE_SIZE, addr, val);
		return L2[addr];
	}
	//Cache miss at L2
	l2cache_miss++;

	//Complete cache miss, so read RAM and update cache
	char val = ram[addr];
	update_cache(L1, L1CACHE_SIZE, addr, val);
	update_Cache(L2, L2CACHE_SIZE, addr, val);
	return val;
}

void write_memory(uword addr, const char* val)
{
	ram[addr] = val;
	
	//Update L1 Cache
	if(in_cache(L1, L1CACHE_SIZE, addr))
	{
		L1[addr] = val;
	}

	//Update L2 Cache
	if(in_cache(L2, L2CACHE_SIZE, addr))
	{
		L2[addr] = val;
	}
}
