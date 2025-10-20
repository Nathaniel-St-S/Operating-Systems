#ifndef PROCESSES_H
#define PROCESSES_H
#include "types.h"
#include "cpu.h"
#include "memory.h"

#define MAX_PROCESSES 5

typedef struct
{
  Cpu state;
} Thread;

typedef struct
{
  int pid;
  mem_addr start;
  mem_addr end;
  int state;
  Thread* threads;
} Process;

//Array to keep track of all the processes
extern Process PROCESS_TABLE[MAX_PROCESSES];

void create_process(int pid);

void init_processes();

void scheduler();

void context_switch(Process current, Process next);
#endif
