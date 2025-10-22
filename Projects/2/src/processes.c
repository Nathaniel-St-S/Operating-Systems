#include "../include/processes.h"
//the index into the process table of the
//current process being run
int current_process = 0;

//the number of active processes, either:
//RUNNING
//WAITING
//READY
int num_proccesses = 0;

//Get the next process
//(round robin style so its just the next index of current)
#define next_process ((current_process + 1) % num_proccesses)

//Self explanatory, initialize the process table with default values
//for no seg fault. (or don't 😈)
void init_processes()
{
  for(int i = 0; i < MAX_PROCESSES; i++)
  {
    PROCESS_TABLE[i].pid = i;
    PROCESS_TABLE[i].cpu_state = CPU;
    PROCESS_TABLE[i].state = READY;
  }
  num_proccesses = MAX_PROCESSES;
  current_process = 0;
  PROCESS_TABLE[0].state = RUNNIG;
}

//Switch from one process to another
//Takes the index of the currnet process and
//the next process to switch to
void context_switch(int current, int next)
{
  Process* curr = &PROCESS_TABLE[current];
  Process* nxt = &PROCESS_TABLE[next];

  //save current's state
  curr->cpu_state = CPU;
  curr->state = READY;

  //start the next process
  CPU = nxt->cpu_state;
  nxt->state = RUNNIG;

  printf("Switched from process (PID: %d) to process (PID: %d)", curr->pid, nxt->pid);
}

//Scheduler that runs each process for a set amount of time
//(round robin style)
//yo @david @brysen ya'll know how to do this
void scheduler()
{
  for(int i = 0; i < num_proccesses; i++)
  {
    if(PROCESS_TABLE[i].state == FINISHED){continue;}
    int time = PROCESS_TIME;
    while(time)
    {
      CPU = PROCESS_TABLE[i].cpu_state;
      fetch();
      execute();
      time--;
    }
    //figure out what to do if a process finishes it's instructions
  }
}
