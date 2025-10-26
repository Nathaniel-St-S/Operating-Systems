#ifndef PROCESSES_H
#define PROCESSES_H
#include "types.h"
#include "cpu.h"
#include "memory.h"

#define MAX_PROCESSES 5
#define PROCESS_TIME 5

typedef enum
{
  READY,
  RUNNING,
  WAITING,
  FINISHED
} ProcessState;

typedef struct
{
  Cpu state;
} Thread;

typedef struct
{
  int pid;
  // dword start_addr;
  // dword end_addr;
  ProcessState state;
  Cpu cpu_state;
  //Thread* threads;
} Process;

//Array to keep track of all the processes
extern Process PROCESS_TABLE[MAX_PROCESSES];

void create_process(int pid);

void init_processes();

void scheduler();

void context_switch(int current, int next);
#endif
