#include "../include/processes.h"
//the index into the process table of the
//current process being run
int current_process = 0;

//the number of active processes, either:
//RUNNING
//WAITING
//READY
int num_processes = 0;

//Get the next process
//(round robin style so its just the next index of current)
#define next_process ((current_process + 1) % num_processes)

Process PROCESS_TABLE[MAX_PROCESSES];

//Self explanatory, initialize the process table with default values
//for no seg fault. (or don't 😈)
void init_processes()
{
  for(int i = 0; i < MAX_PROCESSES; i++)
  {
    PROCESS_TABLE[i].pid = i;
    PROCESS_TABLE[i].cpu_state = THE_CPU;
    PROCESS_TABLE[i].state = READY;
  }
  num_processes = MAX_PROCESSES;
  current_process = 0;
  PROCESS_TABLE[0].state = RUNNING;
}

//Switch from one process to another
//Takes the index of the currnet process and
//the next process to switch to
void context_switch(int current, int next)
{
  Process* curr = &PROCESS_TABLE[current];
  Process* nxt = &PROCESS_TABLE[next];

  //save current's state
  curr->cpu_state = THE_CPU;
  curr->state = READY;

  //start the next process
  THE_CPU = nxt->cpu_state;
  nxt->state = RUNNING;

  printf("Switched from process (PID: %d) to process (PID: %d)", curr->pid, nxt->pid);
}

//Scheduler that runs each process for a set amount of time
//(round robin style)
//yo @david @brysen ya'll know how to do this
void scheduler()
{
  while(true)
  {
    Process* proc = &PROCESS_TABLE[current_process];
    if(proc->state == FINISHED)
    {
      current_process = next_process;
      continue;
    }

    proc->state = RUNNING;
    THE_CPU = proc->cpu_state;

    for(int i = 0; i < PROCESS_TIME; i++)
    {
      fetch();
      execute();
      //check_for_interrupt();

      if(proc->cpu_state.registers[PC] == CPU_HALT)
        break;

    }

    proc->cpu_state = THE_CPU;

    context_switch(current_process, next_process);
    current_process = next_process;

    bool all_done = true;
    for (int j = 0; j < num_processes; j++)
    {
      if (PROCESS_TABLE[j].state != FINISHED)
      {
        all_done = false;
        break;
      }
    }
    if (all_done) break;
  }
}
