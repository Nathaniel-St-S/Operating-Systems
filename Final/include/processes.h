#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdint.h>

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

// I gotta figure our what to do for this
uint32_t makeProcess(int pID, int pc, int priority, int burstTime);

void scheduler(SchedulingAlgorithm algorithm);
#endif
