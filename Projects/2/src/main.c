#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/interrupts.h"
#include "../include/dma.h"
#include "../include/processes.h"
#include <pthread.h>
#include <unistd.h>

void* timer_thread(void* arg) {
    while (1) {
        sleep(1);
        add_interrupt(SAY_HI, 3);
    }
    return NULL;
}

void* io_thread(void* arg) {
    while (1) {
        sleep(2);
        add_interrupt(SAY_GOODBYE, 2);
    }
    return NULL;
}


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
  
  //initialize the process table
  init_processes();

  // TODO - Create a threads for each of the modules

  // Launch asynchronous interrupt sources
  pthread_t timer_thrd, io_thrd;
  pthread_create(&timer_thrd, NULL, timer_thread, NULL);
  pthread_create(&io_thrd, NULL, io_thread, NULL);

  // TODO - execute the cpu and threads in a multithreaded environment

  cpu_run(20, RAM);
  print_cache_stats();
  printf("saved memory value == %X", read_mem(0x000C));

  free_interrupt_controller();
	free(L1.items);
	free(L2.items);
	free(RAM);
  free(HDD);
  free(SSD);

  pthread_cancel(timer_thrd);
  pthread_cancel(io_thrd);
  pthread_join(timer_thrd, NULL);
  pthread_join(io_thrd, NULL);

	return 0;
}
