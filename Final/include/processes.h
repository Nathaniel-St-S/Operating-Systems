#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

// Use INT32_MAX to indicate infinite burst time for interactive programs
#define INFINITE_BURST INT32_MAX

typedef enum {
  SCHED_FCFS,
  SCHED_ROUND_ROBIN,
  SCHED_PRIORITY,
  SCHED_SRT,
  SCHED_HRRN,
  SCHED_SPN,
  SCHED_MLFQ
} SchedulingAlgorithm;

void init_queues(void);

void free_queues(void);

// Reset process storage (for running multiple algorithms in comparison mode)
void reset_process_storage(void);

// Set quiet mode (suppress scheduler debug output)
void set_quiet_mode(bool quiet);

// This serves as a wrapper for the ctx in the assembler pretty
// much. there isn't really another way to make a process out-
// side of this, since all the data is in the assembly. the 
// entry point will initialize the cpu, and the function 
// keeps track of a processes size so that we don't have to
// allocate 1MB for each one. 
//
// delete this once you understand. ask me questions
//
/**
 * Create a process and add it to the new queue
 * 
 * @param pID Process ID
 * @param entry_point Entry point address (where execution starts)
 * @param text_start Start address of text segment in memory
 * @param text_size Size of text segment
 * @param data_start Start address of data segment in memory  
 * @param data_size Size of data segment
 * @param stack_ptr Initial stack pointer value
 * @param priority Process priority (for priority scheduling)
 * @param burstTime Estimated CPU burst time (use INFINITE_BURST for interactive programs)
 * @return Address of allocated memory, or UINT32_MAX on failure
 */
uint32_t makeProcess(int pID, 
                     uint32_t entry_point,
                     uint32_t text_start,
                     uint32_t text_size,
                     uint32_t data_start,
                     uint32_t data_size,
                     uint32_t stack_ptr,
                     int priority, 
                     int burstTime);

void scheduler(SchedulingAlgorithm algorithm);
#endif
