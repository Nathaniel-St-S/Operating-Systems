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
	//initialize the cpu
	init_cpu(&CPU);

	//for later instructions
	write_mem(0x0100, 0x0100);

	//write some random instructions to memory
	write_mem(0x0, 0x5001);
	write_mem(0x1, 0x5002);
	write_mem(0x2, 0x5003);
	write_mem(0x3, 0x5004);
	write_mem(0x4, 0x6004);
	write_mem(0x5, 0x6003);
	write_mem(0x6, 0x6002);
	write_mem(0x7, 0x6001);
	write_mem(0x8, 0x1100);
	write_mem(0x9, 0x5050);
	write_mem(0xA, 0x200C);
	write_mem(0xB, 0xF000);
  
  cpu_run(20, RAM);
  print_cache_stats();
  printf("saved memory value == %X", read_mem(0x000C));

	free(L1.items);
	free(L2.items);
	free(RAM);
	return 0;
}
