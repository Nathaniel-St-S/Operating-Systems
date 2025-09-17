#include "../include/cpu.h"
#include "../include/types.h"
#include "../include/memory.h"
#include "../include/isa.h"

int main()
{
  //initialize the cache for use
	init_cache(&L1, L1CACHE_SIZE);
	init_cache(&L2, L2CACHE_SIZE);
	//initialize the ram
	init_ram(RAM_SIZE);
	write_mem(5, 0x0000);
	write_mem(25, 0xABCD);
	printf("read addr 5  <-> 0x%X\n", read_mem(5));
	printf("read addr 15 <-> 0x%X\n", read_mem(15));
	printf("read addr 5  <-> 0x%X\n", read_mem(5));
	printf("read addr 25 <-> 0x%X\n", read_mem(25));
	printf("read addr 15 <-> 0x%X\n", read_mem(15));
	
	write_mem(5, 0xFFFF);
	printf("read addr 5  <-> 0x%X\n", read_mem(5));

	printf("read addr 30 <-> 0x%X\n", read_mem(30));
	printf("read addr 60 <-> 0x%X\n", read_mem(60));

	print_cache_stats();

	free(L1.items);
	free(L2.items);
	free(RAM);
	return 0;
}
