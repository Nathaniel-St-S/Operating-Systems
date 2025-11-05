#include "../include/memory.h"

/*------------------------------------Global Vars------------------------------------*/

//values for tracking cache stats
int L1cache_hit = 0, L1cache_miss = 0;
int L2cache_hit = 0, L2cache_miss = 0;

//L1 cache
Cache L1;
//L2 cache
Cache L2;
//RAM
dword* RAM = NULL;
//Memory Table
MemoryTable MEMORY_TABLE;
//HDD
dword* HDD = NULL;
//SSD
dword* SSD = NULL;
//working ram size
int WRS;

/*------------------------------------Initializers------------------------------------*/

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

//initialize the memory block
void init_memtable(const int size)
{
  //make the first entry into the table one large free block of memory
  MEMORY_TABLE.blocks = (MemoryBlock*)malloc(sizeof(MemoryBlock) * size);
  MEMBLOCK(0).pid = NO_PID;
  MEMBLOCK(0).is_free = true;
  MEMBLOCK(0).start_addr = 0;
  MEMBLOCK(0).end_addr = (size - 1);

  MEMORY_TABLE.block_count = 1;
  printf("initialized memory table with size %d\n" , size);
}

//initialize the ram to the given size
void init_ram(const int size)
{
  WRS = size;
	RAM = (dword*)malloc(sizeof(dword) * size);
	for(int i = 0; i < size; i++)
	{
		RAM[i] = NO_VAL;
	}
	printf("initialized ram with size: %d\n", size);
  init_memtable(size);
}

//initializes the SSD to the given size 
void init_SSD(const int size) {
	SSD = (dword*)malloc(sizeof(dword) * size);
	for(int i = 0; i < size; i++)
	{
		SSD[i] = NO_VAL;
	}
}

//initializes the HDD to the given size 
void init_HDD(const int size) {
	HDD = (dword*)malloc(sizeof(dword) * size);
	for(int i = 0; i < size; i++)
	{
		HDD[i] = NO_VAL;
	}
}

/*------------------------------------Helper Functions------------------------------------*/

//find the address of the value if it exists in cache
int cache_search(Cache* cache, const dword addr, int* hit_counter, int* miss_counter)
{
	for(int i = 0; i < cache->size; i++)
	{
		//the address we want was found in cache
		//so return the index of that address
		if(cache->items[i].addr == addr)
		{
      (*hit_counter)++;
			return i;
		}
	}
	//address not found so return signifier
  (*miss_counter)++;
	return EMPTY_ADDR;
}
// Searches the L1 cache and updates it's hit/miss counters
static int L1_cache_search(const dword addr) {
  return cache_search(&L1, addr, &L1cache_hit, &L1cache_miss);
}
// Searches the L2 cache and updates it's hit/miss counters
static int L2_cache_search(const dword addr) {
  return cache_search(&L2, addr, &L2cache_hit, &L2cache_miss);
}
// Does a cache search w/o tracking hits or misses
static int cache_search_no_hit_miss(Cache* cache, const dword addr) {
  int x = 0; int* dummy = &x;
  return cache_search(cache, addr, dummy, dummy);
}

//Update the given cache in case of misses
void update_cache(Cache* cache, const dword addr, const dword val)
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

/*------------------------------------External Functions / API------------------------------------*/

//return the value at the given memory address
dword read_mem(const dword addr)
{
	int index;
  // L1 Cache Search
	index = L1_cache_search(addr);
	if(index != EMPTY_ADDR)
		return L1.items[index].val;

  // L2 Cache Search
	index = L2_cache_search(addr);
	if(index != EMPTY_ADDR)
	{
		dword val = L2.items[index].val;
		//Update L1 cache to prevent future cache misses
		update_cache(&L1, addr, val);
		return val;
	}

	//Complete cache miss, so read RAM and update cache
	dword val = RAM[addr];
	update_cache(&L1, addr, val);
	update_cache(&L2, addr, val);
	return val;
}

//write the given value to the given memory address.
void write_mem(const dword addr, const dword val)
{
	RAM[addr] = val;

	int index;
	
	//Update L1 Cache
	index = cache_search_no_hit_miss(&L1, addr);
	if(index != EMPTY_ADDR)
	{
		L1.items[index].val = val;
	}

	//Update L2 Cache
	index = cache_search_no_hit_miss(&L2, addr);
	if(index != EMPTY_ADDR)
	{
		L2.items[index].val = val;
	}
}

// allocate memory for a specific process
// using the best fit method
dword mallocate(int pid, int size)
{
  int best_size = WRS + 1;
  int index = -1;

  for(int i = 0; i < MEMORY_TABLE.block_count; i++)
  {
    if (MEMBLOCK(i).is_free)
    {
      int mem_block_size = (MEMBLOCK(i).end_addr - MEMBLOCK(i).start_addr) + 1;
      if(mem_block_size >= size && mem_block_size < best_size)
      {
        best_size = mem_block_size;
        index = i;
      }
    }
  }

  //No memory free so ...idk
  if(index == -1)
  {
    printf("Could not fullfill process(PID %d)'s request for a (%d byte) chunk of memory: Not Enough Free Space\n", pid, size);
    return -1;
  }

  //Modify the free space to house our process
  //@david Boo! spooky mutation ~~ooooh~~
  MemoryBlock* best_fit = &MEMBLOCK(index);

  //save the old start and end addresses
  dword old_start_addr = best_fit->start_addr;
  dword old_end_addr = best_fit->end_addr;

  dword new_end_addr = (old_start_addr + size) - 1;

  //give the process the space
  best_fit->pid = pid;
  best_fit->is_free = false;
  best_fit->end_addr = new_end_addr;

  //cut down the size of the block
  //to free up unused space
  if(new_end_addr < old_end_addr)
  {
    //shift all the blocks to the right to make room
    for(int i = MEMORY_TABLE.block_count; i > index + 1; i--)
    {
      MEMBLOCK(i) = MEMBLOCK(i - 1);
    }

    MEMBLOCK(index +1).pid = NO_PID;
    MEMBLOCK(index +1).is_free = true;
    MEMBLOCK(index +1).start_addr = new_end_addr + 1;
    MEMBLOCK(index +1).end_addr = old_end_addr;

    MEMORY_TABLE.block_count++;
  }

  printf("Process (PID %d) given (%d bytes) of memory from [%d --> %d]\n", pid, size, best_fit->start_addr, best_fit->end_addr);
  return best_fit->start_addr;
}

// free up the memory block associated with the process
void liberate(int pid)
{
  int index = 0;
  for(index = 0; index < MEMORY_TABLE.block_count; index++)
  {
    if(MEMBLOCK(index).pid == pid && !MEMBLOCK(index).is_free)
    {
      MEMBLOCK(index).pid = NO_PID;
      MEMBLOCK(index).is_free = true;
      printf("Freed (PID %d) at memory [%d --> %d]\n", pid, MEMBLOCK(index).start_addr, MEMBLOCK(index).end_addr);
      break;
    }
  }
  //if the pid was not found
  if(index == MEMORY_TABLE.block_count){return;}

  //merge newly freed block with the previous block if it's also free
  if(index > 0 && MEMBLOCK(index - 1).is_free)
  {
    MEMBLOCK(index -1).end_addr = MEMBLOCK(index).end_addr;
    //shift everything left to clean the gap
    for(int i = index; i < MEMORY_TABLE.block_count - 1; i++)
    {
      MEMBLOCK(i) = MEMBLOCK(i + 1);
    }
    MEMORY_TABLE.block_count--;
    index--;
  }

  //merge with the next memory block if it's also free
  if(index < MEMORY_TABLE.block_count - 1 && MEMBLOCK(index + 1).is_free)
  {
    MEMBLOCK(index).end_addr = MEMBLOCK(index + 1).end_addr;
    //shift right to clean the gap
    for(int i = index + 1; i < MEMORY_TABLE.block_count - 1; i++)
    {
      MEMBLOCK(i) = MEMBLOCK(i + 1);
    }
    MEMORY_TABLE.block_count--;
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
