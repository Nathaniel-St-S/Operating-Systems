#include "../include/processes.h"
#include "../include/cpu.h"
#include "../include/memory.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------Structs & Enums-------------------------------------//
typedef enum {
  NORMAL,
  PRIORITYBURST,
  PRIORITYPRIORITY
} QueueTypeEnum;

typedef enum {
  READY,
  RUNNING,
  SUSPEND_READY,
  BLOCKED,
  SUSPEND_BLOCKED,
  NEW,
  FINISHED
} ProcessState;

//To represent a process
typedef struct {
  int pid; //process id
  int pc; //program counter 
  ProcessState state; //state of the process
  int priority; //priority level
  int burstTime; //time left to complete
  float responseRatio; //calculated as (waiting time + service time) //service time
  Cpu cpu_state; 
} Process;

//To represent a queue
typedef struct {
  int next; //the index to next open space
  int capacity; //The size of the queue
  Process PCB[]; //The block to hold processes
} Queue;

//-------------------------------------Constants-------------------------------------//
#define QUANTUM 3
#define MAX_PROCESSES 10

static Queue* Ready_Queue = NULL;
static Queue* Running_Queue = NULL;
static Queue* Blocked_Queue = NULL;
static Queue* Suspend_Blocked_Queue = NULL;
static Queue* Suspend_Ready_Queue = NULL;
static Queue* New_Queue = NULL;
static Queue* Finished_Queue = NULL;

//-------------------------------------Initializers for Queue-------------------------------------//

static void init_Ready_Queue(const int size) {
  Ready_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!Ready_Queue) {
    perror("calloc Ready_Queue");
    exit(EXIT_FAILURE);
  }
  Ready_Queue->next = 0;
  Ready_Queue->capacity = size;
}

static void init_Running_Queue(const int size) {
  Running_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!Running_Queue) {
    perror("calloc Running_Queue");
    exit(EXIT_FAILURE);
  }
  Running_Queue->next = 0;
  Running_Queue->capacity = size;
}

static void init_Blocked_Queue(const int size) {
  Blocked_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!Blocked_Queue) {
    perror("calloc Blocked_Queue");
    exit(EXIT_FAILURE);
  }
  Blocked_Queue->next = 0;
  Blocked_Queue->capacity = size;
}

static void init_Suspend_Blocked_Queue(const int size) {
  Suspend_Blocked_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!Suspend_Blocked_Queue) {
    perror("calloc Suspend_Blocked_Queue");
    exit(EXIT_FAILURE);
  }
  Suspend_Blocked_Queue->next = 0;
  Suspend_Blocked_Queue->capacity = size;
}

static void init_Suspend_Ready_Queue(const int size) {
  Suspend_Ready_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!Suspend_Ready_Queue) {
    perror("calloc Suspend_Ready_Queue");
    exit(EXIT_FAILURE);
  }
  Suspend_Ready_Queue->next = 0;
  Suspend_Ready_Queue->capacity = size;
}

static void init_New_Queue(const int size) {
  New_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!New_Queue) {
    perror("calloc New_Queue");
    exit(EXIT_FAILURE);
  }
  New_Queue->next = 0;
  New_Queue->capacity = size;
}

static void init_Finished_Queue(const int size) {
  Finished_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!Finished_Queue) {
    perror("calloc Finished_Queue");
    exit(EXIT_FAILURE);
  }
  Finished_Queue->next = 0;
  Finished_Queue->capacity = size;
}

Queue* init_FeedBack_Queue(const int size) {
  Queue* FeedBack_Queue = calloc(1, sizeof(Queue) + (size_t)size * sizeof(Process));
  if (!FeedBack_Queue) {
    perror("calloc FeedBack_Queue");
    exit(EXIT_FAILURE);
  }
  FeedBack_Queue->next = 0;
  FeedBack_Queue->capacity = size;
  return FeedBack_Queue;
}

void init_queues(void) {
  init_Ready_Queue(MAX_PROCESSES);
  init_Running_Queue(MAX_PROCESSES);
  init_Blocked_Queue(MAX_PROCESSES);
  init_Suspend_Blocked_Queue(MAX_PROCESSES);
  init_Suspend_Ready_Queue(MAX_PROCESSES);
  init_New_Queue(MAX_PROCESSES);
  init_Finished_Queue(MAX_PROCESSES);
}

static void free_Queue(Queue* Q) {
  free(Q->PCB);
  free(Q);
}

void free_queues(void){
  free_Queue(Ready_Queue);
  free_Queue(Running_Queue);
  free_Queue(Blocked_Queue);
  free_Queue(Suspend_Blocked_Queue);
  free_Queue(Suspend_Ready_Queue);
  free_Queue(New_Queue);
  free_Queue(Finished_Queue);
}

//-------------------------------------Helpers for Queue-------------------------------------//

//for non priority queues
static void enqueueGeneric(Process elem, Queue* Q) {
  if (Q == Finished_Queue && Q->next >= Q->capacity) {
    Q->next = 0;
  }

  if (Q->next >= Q->capacity) {
    fprintf(stderr, "Queue is full\n");
    return;
  }

  Q->PCB[Q->next] = elem;
  Q->next+=1;
}

//for non priority queues
static Process dequeueGeneric(Queue* Q) {
  if (Q->next == 0) {
    fprintf(stderr, "Queue is empty\n");
    Process empty = {0};
    return empty;
  }

  int i = 0;
  Process process = Q->PCB[0];
  Q->next-=1;
  while (i < Q->next) {
    Q->PCB[i] = Q->PCB[i+1];
    i+=1;
  }

  return process;
}

// Define swap function to swap two PCB
static void swap(Queue* Q, int swappee, int swapper) {
  Process temp = Q->PCB[swappee];
  Q->PCB[swappee] = Q->PCB[swapper];
  Q->PCB[swapper] = temp;
}

// Adds a procces to the priority queue WRT to a process's burst time
static void enqueueBurst(Process process, Queue* Q) {
  if (Q->next >= Q->capacity) {
    fprintf(stderr, "Priority queue is full\n");
    return;
  }

  int index = Q->next++;
  Q->PCB[index] = process;
  while (index != 0 && Q->PCB[(index - 1) / 2].burstTime > Q->PCB[index].burstTime) {
    swap(Q, index, (index - 1) / 2);
    index = (index - 1) / 2;
  }
}

// Extracts a process and maintains the priority queue WRT burst time
static Process dequeueBurst(Queue* Q) {
  if (Q->next == 0) {
    fprintf(stderr, "Priority queue is empty\n");
    Process empty = {0};
    return empty;
  }

  Process process = Q->PCB[0];

  Q->PCB[0] = Q->PCB[--Q->next];
  int index = 0;
  while(true) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < Q->next && Q->PCB[left].burstTime < Q->PCB[smallest].burstTime) {
      smallest = left;
    }

    if (right < Q->next && Q->PCB[right].burstTime < Q->PCB[smallest].burstTime) {
      smallest = right;
    }

    if (smallest != index) {
      swap(Q, index, smallest);
      index = smallest;
    } else {
      break;
    }
  }
  return process;
}


// Adds a procces to the priority queue WRT to a process's priority
static void enqueuePriority(Process process, Queue* Q) {
  if (Q->next >= Q->capacity) {
    fprintf(stderr, "Priority queue is full\n");
    return;
  }

  int index = Q->next++;
  Q->PCB[index] = process;
  while (index != 0 && Q->PCB[(index - 1) / 2].priority > Q->PCB[index].priority) {
    swap(Q, index, (index - 1) / 2);
    index = (index - 1) / 2;
  }
}

// Extracts a process and maintains the priority queue WRT priority
static Process dequeuePriority(Queue* Q) {
  if (Q->next == 0) {
    fprintf(stderr, "Priority queue is empty\n");
    Process empty = {0};
    return empty;
  }

  Process process = Q->PCB[0];

  Q->PCB[0] = Q->PCB[--Q->next];
  int index = 0;
  while(true) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < Q->next && Q->PCB[left].priority < Q->PCB[smallest].priority) {
      smallest = left;
    }

    if (right < Q->next && Q->PCB[right].priority < Q->PCB[smallest].priority) {
      smallest = right;
    }

    if (smallest != index) {
      swap(Q, index, smallest);
      index = smallest;
    } else {
      break;
    }
  }
  return process;
}

//enqueues the process depending on the queue type
static void enqueueHelper(Process P, int queue_type) {
  switch(queue_type) {
    case NORMAL: enqueueGeneric(P, Ready_Queue); break;
    case PRIORITYBURST: enqueueBurst(P, Ready_Queue); break;
    case PRIORITYPRIORITY: enqueuePriority(P, Ready_Queue); break;
    default: fprintf(stderr, "Unknown Queue Type\n"); break;
  }
}

// Enqueues the process depending the process's state
static void enqueue(Process P, int queue_type) {
  switch (P.state) {
    case READY: enqueueHelper(P, queue_type); break; 
    case BLOCKED: enqueueGeneric(P, Blocked_Queue); break;
    case SUSPEND_READY: enqueueGeneric(P, Suspend_Ready_Queue); break;
    case SUSPEND_BLOCKED: enqueueGeneric(P, Suspend_Blocked_Queue); break;
    case RUNNING: enqueueGeneric(P, Running_Queue); break;
    case NEW: enqueueGeneric(P, New_Queue); break;
    case FINISHED: enqueueGeneric(P, Finished_Queue); break; 
    default: fprintf(stderr, "Unknown Process State\n"); break;  
  }
}

// Returns the process at the front of the queue depending on the queue type
static Process dequeue(Queue* Q, int queue_type) {
  Process P;
  switch(queue_type) {
    case NORMAL: P = dequeueGeneric(Q); break;
    case PRIORITYBURST: P = dequeueBurst(Q); break;
    case PRIORITYPRIORITY: P = dequeuePriority(Q); break;
    default: P = (Process){0}; break;
  } 
  return P;
}

//-------------------------------------State Transitions-------------------------------------//

//transitions a process 's state from one to another and moves to its corresponding queue
static void transitionState(Process P, int queue_type) {
  if (P.burstTime <= 0) {
    P.state = FINISHED; 
    enqueue(dequeue(Running_Queue, NORMAL), NORMAL);
  } else if ((P.state == NEW) || (P.burstTime > 0 && RUNNING)) {
    P.state = READY;
    if (P.state == NEW) {
      enqueue(dequeue(New_Queue, queue_type), queue_type);
    } else {
      enqueue(dequeue(Running_Queue, queue_type), queue_type);
    }
  } else {
    P.state = RUNNING;
    enqueue(dequeue(Ready_Queue, NORMAL), NORMAL);
  }
}

//-------------------------------------Process Creation-------------------------------------//

static Process global_process_storage[10];
static int process_storage_index = 0;

uint32_t makeProcess(int pID, int pc, int priority, int burstTime) {
  if (process_storage_index >= 10) {
    fprintf(stderr, "Too many processes created\n");
    return UINT32_MAX;
  }

  Process* newProcess = &global_process_storage[process_storage_index++];
  newProcess->pid = pID;
  newProcess->pc = pc;
  newProcess->state = NEW;
  newProcess->priority = priority;
  newProcess->burstTime = burstTime;
  newProcess->responseRatio = 0;
  newProcess->cpu_state = THE_CPU;

  uint32_t address = mallocate(pID, MAX_PROCESS_SIZE);
  if(address == UINT32_MAX)
    fprintf(stderr, "Could not create new process of pID %d\n", pID);
  enqueue(*newProcess, NORMAL);
  return address;
}

//-------------------------------------Context Switching-------------------------------------//

//Switches from one process to another
static void context_switch(int queue_type, bool needTransition) {
  Process curr = Running_Queue->PCB[0];
  Process nxt = Ready_Queue->PCB[0];

  //save current's state
  curr.cpu_state = THE_CPU;
  //start the next process
  THE_CPU = nxt.cpu_state;

  if (needTransition) {
    transitionState(curr, queue_type);
    transitionState(nxt, queue_type);
  }

  printf("Switched from process (PID: %d) to process (PID: %d)", curr.pid, nxt.pid);
}

//-------------------------------------Scheduling Helpers-------------------------------------//

//returns the process with the highest response ratio
static Process getHighestResponseRatio(void) {
  int i = 1; 
  Process HRRProcess = Ready_Queue->PCB[0];
  while (i < Ready_Queue->next) {
    if (Ready_Queue->PCB[i].responseRatio < HRRProcess.responseRatio) {
      HRRProcess = Ready_Queue->PCB[i];
    }
    i+=1;
  }
  swap(Ready_Queue, 0, 1);
  return HRRProcess;
}

//calculates given process's response ratio
static float calcResponseRatio(Process P, int idleTime) {
  if (P.burstTime == 0) return 0.0f;
  return (float)(idleTime + P.burstTime) / (float)P.burstTime;
}

//updates the response ratio for every process in the given queue
static void updateResponseRatio(Queue* Q, int idleTime) {
  int i = 0;
  while (i < Q->next) {
    Q->PCB[i].responseRatio = calcResponseRatio(Q->PCB[i], idleTime);
    i++;
  }
}

//transfers all the processes in the new queue to the its respective ready queue
static void transferProcesses(int queue_type) {
  while (New_Queue->next != 0) {
    enqueue(dequeue(New_Queue, queue_type), queue_type);
  }
}

//-------------------------------------Scheduling Algorithms-------------------------------------//

//the round robin scheduling algorithm
static void roundRobin(void) {
  int idx = 0;
  while (Ready_Queue->next != 0) {
    transferProcesses(NORMAL);
    Process currentProcess = Ready_Queue->PCB[idx];
    transitionState(currentProcess, NORMAL);
    for (int i = 0; i < QUANTUM; i++) {
      fetch();
      execute();
      currentProcess.burstTime-=1; //could make ternary
    }

    if (currentProcess.burstTime > 0 && idx+1 >= Ready_Queue->next) {
      context_switch(NORMAL, true);
      idx = 0;
    } else if (currentProcess.burstTime > 0) {
      context_switch(NORMAL, true);
      idx+=1;
    } else {
      transitionState(currentProcess, NORMAL);
    }

  }
}


//The first come first serve scheduling algorithm
static void firstComeFirstServe(void) {
  while (Ready_Queue->next != 0) {
    transferProcesses(NORMAL);
    Process currentProcess = Ready_Queue->PCB[0];
    transitionState(currentProcess, NORMAL);
    while (currentProcess.burstTime > 0) {
      fetch();
      execute();
      currentProcess.burstTime-=1;
    }
    transitionState(currentProcess, NORMAL);
  }
}

//uses priorityBurstQueue 
//the shortest process next scheduling algorithm
static void shortestProcessNext(void) {
  while (Ready_Queue->next != 0) {
    transferProcesses(PRIORITYBURST);
    Process shortestProcess = Ready_Queue->PCB[0]; //searchForShortestProcess(Ready_Queue);
    transitionState(shortestProcess, PRIORITYBURST);
    while(shortestProcess.burstTime > 0) {
      fetch();
      execute();
      shortestProcess.burstTime-=1;
    }
    transitionState(shortestProcess, PRIORITYBURST); 
  }
}

//uses priorityPriorityQueue 
//the priority based scheduling algorithm
static void priorityBased(void) {
  while (Ready_Queue->next != 0) {
    transferProcesses(PRIORITYPRIORITY);
    Process highestPriorityP = Ready_Queue->PCB[0];
    transitionState(highestPriorityP, PRIORITYPRIORITY);
    while (highestPriorityP.burstTime > 0) {
      fetch();
      execute();
      highestPriorityP.burstTime-=1;
      transferProcesses(PRIORITYPRIORITY);
      Process newHighestP = Ready_Queue->PCB[0];

      if (&newHighestP != &highestPriorityP) {
        if (&Ready_Queue->PCB[1] == &newHighestP) {
          context_switch(PRIORITYPRIORITY, true);
        } else {
          context_switch(PRIORITYPRIORITY, true);
        }
      }
    }
    transitionState(highestPriorityP, PRIORITYPRIORITY);
  }
}
//uses priorityBurstQueue 
//the shortest time remaining scheduling algorithm
static void shortestRemainingTime(void) {
  while (Ready_Queue->next != 0) {
    transferProcesses(PRIORITYBURST);
    Process shortestBTimeP = Ready_Queue->PCB[0];
    transitionState(shortestBTimeP, PRIORITYBURST);
    while (shortestBTimeP.burstTime > 0) {
      fetch();
      execute();
      shortestBTimeP.burstTime-=1;
      transferProcesses(PRIORITYBURST);
      Process newShortestBTimeP = Ready_Queue->PCB[0];

      if (&newShortestBTimeP != &shortestBTimeP) {
        if (&Ready_Queue->PCB[1] == &newShortestBTimeP) {
          context_switch(PRIORITYBURST, true);
        } else {
          context_switch(PRIORITYBURST, true);
        }
      }
    }
    transitionState(shortestBTimeP, PRIORITYBURST);
  }
}

//the highest response ratio next scheduling algorithm
static void highestResponseRatioNext(void) {
  transferProcesses(NORMAL);
  if (Ready_Queue->next != 0) {
    int total_time = 0;
    Process currentProcess = Ready_Queue->PCB[0];
    total_time = currentProcess.burstTime;
    transitionState(currentProcess, NORMAL);
    while (Ready_Queue->next != 0) {
      transferProcesses(NORMAL);
      while (currentProcess.burstTime > 0) {
        fetch();
        execute();
        currentProcess.burstTime-=1;
      }

      transitionState(currentProcess, NORMAL);
      updateResponseRatio(Ready_Queue, total_time); 
      currentProcess = getHighestResponseRatio();
      total_time = currentProcess.burstTime;
      transitionState(currentProcess, NORMAL);
    }
  }
}

//the feedback scheduling algorithm
static void feedBack(void) {
  Queue* feedBack_Q2 = init_FeedBack_Queue(MAX_PROCESSES);
  Queue* feedBack_Q3 = init_FeedBack_Queue(MAX_PROCESSES);
  int quantum1 = 2;
  int quantum2 = 4;

  while(true) {
    transferProcesses(NORMAL);
    //handle processes in highest priority queue with 2 quantum
    if (Ready_Queue->next != 0) {
      Process P = Ready_Queue->PCB[0];
      transitionState(P, NORMAL);
      transferProcesses(NORMAL);
      for (int i = 0; i < quantum1 && P.burstTime > 0; i++) {
        fetch();
        execute();
        P.burstTime-=1;
      }
    }

    //handle processes in middle priority queue with 4 quantum
    if (feedBack_Q2->next != 0) {
      Process P = feedBack_Q2->PCB[0];
      transitionState(P, NORMAL);
      transferProcesses(NORMAL);
      for (int j = 0; j < quantum2 && P.burstTime > 0; j++) {
        fetch();
        execute();
        P.burstTime-=1;
      }
    }

    //handle processes in lowest priority queue FCFS
    if (feedBack_Q3->next != 0) {
      Process P = feedBack_Q3->PCB[0];
      transitionState(P, NORMAL);
      transferProcesses(NORMAL);
      while (P.burstTime > 0) {
        fetch();
        execute();
        P.burstTime-=1;
      }
    }

    //highest priority queue not empty
    if (Ready_Queue->next != 0) {
      //move process from Ready queue to finished queue
      if (Ready_Queue->PCB[0].burstTime <= 0) {
        transitionState(Ready_Queue->PCB[0], NORMAL);
      } else if (Ready_Queue->next >= 2) { // switch context then move to middle priority queue
        context_switch(NORMAL, false);
        enqueueGeneric(dequeue(Ready_Queue, NORMAL), feedBack_Q2);
      } else { // move to middle priority queue
        enqueueGeneric(dequeue(Ready_Queue, NORMAL), feedBack_Q2);
      }
    }

    //middle priority queue not empty
    if (feedBack_Q2->next != 0) {
      //move process from middle priority queue to finished queue
      if (feedBack_Q2->PCB[0].burstTime <= 0) {
        transitionState(feedBack_Q2->PCB[0], NORMAL);
      } else if (feedBack_Q2->next >= 2) { // switch context then move to lowest priority queue
        context_switch(NORMAL, false);
        enqueueGeneric(dequeue(feedBack_Q2, NORMAL), feedBack_Q3);
      } else { // move to lowest priority queue
        enqueueGeneric(dequeue(feedBack_Q2, NORMAL), feedBack_Q3);
      }
    }

    //lowest priority queue not empty
    if (feedBack_Q3->next != 0) {
      //move process from middle priority queue to finished queue
      if (feedBack_Q3->PCB[0].burstTime <= 0) {
        transitionState(feedBack_Q3->PCB[0], NORMAL);
      }
    }

    if (Ready_Queue->next == 0 && feedBack_Q2->next == 0 && feedBack_Q3->next == 0) {
      break;
    }
  }

  free_Queue(feedBack_Q2);
  free_Queue(feedBack_Q3);
}

//-------------------------------------Scheduler-------------------------------------//

// To start the scheduler with the given type
void scheduler(SchedulingAlgorithm algorithm) {

  switch (algorithm) {
    case SCHED_ROUND_ROBIN: roundRobin(); break;
    case SCHED_PRIORITY: priorityBased(); break;
    case SCHED_SRT: shortestRemainingTime(); break;
    case SCHED_HRRN: highestResponseRatioNext(); break;
    case SCHED_FCFS: firstComeFirstServe(); break;
    case SCHED_SPN: shortestProcessNext(); break;
    case SCHED_MLFQ: feedBack(); break;
    default: fprintf(stderr, "Unknown Scheduler Type\n"); break; 
  }
}
