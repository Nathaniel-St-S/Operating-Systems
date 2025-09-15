#include <stdio.h>
#include <stdlib.h>
#include "../include/types.h"

#define L1CACHE_SIZE 64000
#define L2CACHE_SIZE 1000000
#define RAM_SIZE 5000000000
#define HEX_SIZE 5

//Return the value at the given memory adress
uword read_mem(uword addr);

//Write given value to the given adress, 
void write_mem(uword addr, uword val);

//check if the given adress is in the given cache
bool in_cache(uword* cache, int cache_size, uword addr);

//Update the given cache in case of misses
void update_cache(uword* cache, int cache_size, uword addr, uword val);
	
//uword* L1 = (uword*)malloc(sizeof(uword) * L1CACHE_SIZE);
//uword* L2 = (uword*)malloc(sizeof(uword) * L2CACHE_SIZE);
//uword* RAM = (uword*)malloc(sizeof(uword) * RAM_SIZE)
uword L1[L1CACHE_SIZE];
uword L2[L2CACHE_SIZE];
uword RAM[RAM_SIZE];

//Track the numbur of misses for stats
int L1cache_hit = 0, L1cache_miss = 0;
int L2cache_hit = 0, L2cache_miss = 0;

int main()
{
	printf("read addr 5 <-> %s/n", read_mem(5));
	printf("read addr 15 <-> %s/n", read_mem(15));
	printf("read addr 5 <-> %s/n", read_mem(5));
	printf("read addr 25 <-> %s/n", read_mem(25));
	printf("read addr 15 <-> %s/n", read_mem(15));
	
	write_mem(5, 0xFFFF);
	printf("read addr 5 <-> %s/n", read_mem(5));

	printf("read addr 30 <-> %s/n", read_mem(30));
	printf("read addr 60 <-> %s/n", read_mem(60));

	return 0;
}

bool in_cache(uword* cache, int cache_size, uword addr)
{
	for(int i = 0; i < cache_size; i++)
	{
		if(cache[i] == addr)
		{
			return true;
		}
	}

	return false;
}

uword read_mem(uword addr)
{
	if(in_cache(L1, L1CACHE_SIZE, addr))
	{
		//Cache hit at l1
		L1cache_hit++;
		return L1[addr];
	}

	//cache miss at L1
	L1cache_miss++;

	if(in_cache(L2, L2CACHE_SIZE, addr))
	{
		//Cache hit at l2
		L2cache_hit++;
		//Update L1 cache to prevent future cache misses
		update_cache(L1, L1CACHE_SIZE, addr, val);
		return L2[addr];
	}
	//Cache miss at L2
	L2cache_miss++;

	//Complete cache miss, so read RAM and update cache
	uword val = RAM[addr];
	update_cache(L1, L1CACHE_SIZE, addr, val);
	update_cache(L2, L2CACHE_SIZE, addr, val);
	return val;
}

void write_memory(uword addr, const uword val)
{
	RAM[addr] = val;
	
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
