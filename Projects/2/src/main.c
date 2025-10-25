#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/interrupts.h"
#include "../include/dma.h"

int main()
{
  //initialize the cache for use
	init_cache(&L1, L1CACHE_SIZE);
	init_cache(&L2, L2CACHE_SIZE);

	//initialize the ram
	init_ram(RAM_SIZE);
	init_HDD(HDD_SIZE);
	init_SSD(SSD_SIZE);

  //initialize the interrupt controller
  init_interrupt_controller();

  //initialize the cpu
	init_cpu(&THE_CPU);

  //memory adresses for later
  write_mem(0x0100, 0x0100);

	//write some random instructions to memory
	write_mem(0x0, 0x5001);
	write_mem(0x1, 0x5002);
	write_mem(0x2, 0x5003);

	initiateDMA(RAM, HDD, 100);

	write_mem(0x3, 0x5004);
	write_mem(0x4, 0x6004);
	write_mem(0x5, 0x6003);

  add_interrupt(0x0001, 5);

	write_mem(0x6, 0x6002);
	write_mem(0x7, 0x6001);

	initiateDMA(HDD, SSD, 25);

	write_mem(0x8, 0x1100);
	write_mem(0x9, 0x5050);

	initiateDMA(RAM, SSD, 50);

	write_mem(0xA, 0x200C);

  add_interrupt(0x0002, 1);

	initiateDMA(SSD, HDD, 200);

	write_mem(0xB, 0xF000);
	
  
  cpu_run(20, RAM);
  print_cache_stats();
  printf("saved memory value == %X", read_mem(0x000C));

  free_interrupt_controller();
	free(L1.items);
	free(L2.items);
	free(RAM);
  free(HDD);
  free(SSD);
	return 0;
}
