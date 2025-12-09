#include "../include/processes.h"
#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/isa.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  int pid;
  uint32_t pc;
  ProcessState state;
  int priority;
  int burstTime;
  float responseRatio;
  Cpu cpu_state;
  uint32_t text_start;
  uint32_t text_size;
  uint32_t data_start;
  uint32_t data_size;
  uint32_t stack_ptr;
} Process;

//To represent a queue
typedef struct {
  int next;
  int capacity;
  Process PCB[];
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

static void shift_ready_left(void) {
  if (Ready_Queue->next == 0) return;
  for (int i = 0; i + 1 < Ready_Queue->next; i++) {
    Ready_Queue->PCB[i] = Ready_Queue->PCB[i + 1];
  }
  Ready_Queue->next--;
}

static void push_ready_back(Process p) {
  if (Ready_Queue->next >= Ready_Queue->capacity) {
    fprintf(stderr, "Ready queue full, dropping PID %d\n", p.pid);
    return;
  }
  Ready_Queue->PCB[Ready_Queue->next++] = p;
}

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
  if (!Q) {
    return;
  }
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

static Process dequeueGeneric(Queue* Q) {
  if (Q->next == 0) {
    fprintf(stderr, "Queue is empty\n");
    Process empty = {0};
    return empty;
  }

  Process process = Q->PCB[0];
  Q->next-=1;
  for (int i = 0; i < Q->next; i++) {
    Q->PCB[i] = Q->PCB[i+1];
  }

  return process;
}

static void swap(Queue* Q, int swappee, int swapper) {
  Process temp = Q->PCB[swappee];
  Q->PCB[swappee] = Q->PCB[swapper];
  Q->PCB[swapper] = temp;
}

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

static void enqueueHelper(Process P, int queue_type) {
  switch(queue_type) {
    case NORMAL: enqueueGeneric(P, Ready_Queue); break;
    case PRIORITYBURST: enqueueBurst(P, Ready_Queue); break;
    case PRIORITYPRIORITY: enqueuePriority(P, Ready_Queue); break;
    default: fprintf(stderr, "Unknown Queue Type\n"); break;
  }
}

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

//-------------------------------------Scheduling Helpers-------------------------------------//

static void transferProcesses(int queue_type) {
  while (New_Queue->next != 0) {
    Process p = dequeue(New_Queue, NORMAL);
    p.state = READY;
    enqueueHelper(p, queue_type);
  }
}

static float calcResponseRatio(Process P, int idleTime) {
  if (P.burstTime == 0) return 0.0f;
  return (float)(idleTime + P.burstTime) / (float)P.burstTime;
}

static void updateResponseRatio(Queue* Q, int idleTime) {
  for (int i = 0; i < Q->next; i++) {
    Q->PCB[i].responseRatio = calcResponseRatio(Q->PCB[i], idleTime);
  }
}

static int getHighestResponseRatioIndex(void) {
  if (Ready_Queue->next == 0) return -1;
  
  int best = 0;
  for (int i = 1; i < Ready_Queue->next; i++) {
    if (Ready_Queue->PCB[i].responseRatio > Ready_Queue->PCB[best].responseRatio) {
      best = i;
    }
  }
  return best;
}

//-------------------------------------Process Creation-------------------------------------//

static Process global_process_storage[MAX_PROCESSES];
static int process_storage_index = 0;

uint32_t makeProcess(int pID, 
                     uint32_t entry_point,
                     uint32_t text_start,
                     uint32_t text_size,
                     uint32_t data_start,
                     uint32_t data_size,
                     uint32_t stack_ptr,
                     int priority, 
                     int burstTime) {
  
  if (process_storage_index >= MAX_PROCESSES) {
    fprintf(stderr, "makeProcess: Too many processes (max %d)\n", MAX_PROCESSES);
    return UINT32_MAX;
  }

  if (text_start == UINT32_MAX) {
    fprintf(stderr, "makeProcess: Invalid text_start address for PID %d\n", pID);
    return UINT32_MAX;
  }

  Process* newProcess = &global_process_storage[process_storage_index++];
  
  newProcess->pid = pID;
  newProcess->pc = entry_point;
  newProcess->state = NEW;
  newProcess->priority = priority;
  newProcess->burstTime = burstTime;
  newProcess->responseRatio = 0;
  
  newProcess->text_start = text_start;
  newProcess->text_size = text_size;
  newProcess->data_start = data_start;
  newProcess->data_size = data_size;
  newProcess->stack_ptr = stack_ptr;
  
  memset(&newProcess->cpu_state, 0, sizeof(Cpu));
  
  newProcess->cpu_state.hw_registers[PC] = entry_point;
  newProcess->cpu_state.gp_registers[REG_SP] = stack_ptr;
  newProcess->cpu_state.gp_registers[REG_GP] = data_start;
  newProcess->cpu_state.gp_registers[REG_ZERO] = 0;
  
  enqueue(*newProcess, NORMAL);
  
  printf("  ✓ Process created:\n");
  printf("      PID: %d\n", pID);
  printf("      PC:  0x%08x\n", entry_point);
  printf("      SP:  0x%08x\n", stack_ptr);
  printf("      Text: 0x%08x - 0x%08x (%u bytes)\n", 
         text_start, text_start + text_size, text_size);
  if (data_size > 0) {
    printf("      Data: 0x%08x - 0x%08x (%u bytes)\n", 
           data_start, data_start + data_size, data_size);
  }
  printf("      Priority: %d, Burst: %d\n", priority, burstTime);
  
  return entry_point;
}

//-------------------------------------Scheduling Algorithms-------------------------------------//

static void roundRobin(void) {
  transferProcesses(NORMAL);

  while (Ready_Queue->next > 0) {
    Process *p = &Ready_Queue->PCB[0];

    set_current_process(p->pid);
    THE_CPU = p->cpu_state;

    int slice = (p->burstTime < QUANTUM) ? p->burstTime : QUANTUM;
    for (int i = 0; i < slice; i++) {
      if (THE_CPU.hw_registers[PC] == CPU_HALT) break;
      fetch();
      execute();
      if (p->burstTime > 0) {
        p->burstTime--;
      }
    }

    p->cpu_state = THE_CPU;

    bool finished = (p->burstTime <= 0) || (THE_CPU.hw_registers[PC] == CPU_HALT);
    if (finished) {
      printf("Process %d finished - freeing memory\n", p->pid);
      liberate(p->pid);
      shift_ready_left();
    } else {
      Process tmp = *p;
      shift_ready_left();
      push_ready_back(tmp);
    }
  }

  set_current_process(SYSTEM_PROCESS_ID);
}

static void firstComeFirstServe(void) {
  transferProcesses(NORMAL);
  
  while (Ready_Queue->next > 0) {
    Process *p = &Ready_Queue->PCB[0];
    set_current_process(p->pid);
    THE_CPU = p->cpu_state;
    
    while (p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT) {
      fetch();
      execute();
      p->burstTime--;
    }
    
    p->cpu_state = THE_CPU;
    printf("Process %d finished - freeing memory\n", p->pid);
    liberate(p->pid);
    shift_ready_left();
  }
  
  set_current_process(SYSTEM_PROCESS_ID);
}

static void shortestProcessNext(void) {
  transferProcesses(PRIORITYBURST);
  
  while (Ready_Queue->next > 0) {
    Process *p = &Ready_Queue->PCB[0];
    set_current_process(p->pid);
    THE_CPU = p->cpu_state;
    
    while(p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT) {
      fetch();
      execute();
      p->burstTime--;
    }
    
    p->cpu_state = THE_CPU;
    printf("Process %d finished - freeing memory\n", p->pid);
    liberate(p->pid);
    dequeueGeneric(Ready_Queue); // Remove from heap
    transferProcesses(PRIORITYBURST);
  }
  
  set_current_process(SYSTEM_PROCESS_ID);
}

static void priorityBased(void) {
  transferProcesses(PRIORITYPRIORITY);
  
  while (Ready_Queue->next > 0) {
    Process *p = &Ready_Queue->PCB[0];
    set_current_process(p->pid);
    THE_CPU = p->cpu_state;
    
    while (p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT) {
      fetch();
      execute();
      p->burstTime--;
      
      transferProcesses(PRIORITYPRIORITY);
      
      // Check if higher priority process arrived
      if (Ready_Queue->next > 1 && Ready_Queue->PCB[1].priority < p->priority) {
        p->cpu_state = THE_CPU;
        Process tmp = *p;
        dequeueGeneric(Ready_Queue); // Remove current from heap
        enqueuePriority(tmp, Ready_Queue); // Re-insert
        break;
      }
    }
    
    bool finished = (p->burstTime <= 0) || (THE_CPU.hw_registers[PC] == CPU_HALT);
    if (finished) {
      p->cpu_state = THE_CPU;
      printf("Process %d finished - freeing memory\n", p->pid);
      liberate(p->pid);
      dequeueGeneric(Ready_Queue);
    }
  }
  
  set_current_process(SYSTEM_PROCESS_ID);
}

static void shortestRemainingTime(void) {
  transferProcesses(PRIORITYBURST);
  
  while (Ready_Queue->next > 0) {
    Process *p = &Ready_Queue->PCB[0];
    set_current_process(p->pid);
    THE_CPU = p->cpu_state;
    
    while (p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT) {
      fetch();
      execute();
      p->burstTime--;
      
      transferProcesses(PRIORITYBURST);
      
      // Check if shorter process arrived
      if (Ready_Queue->next > 1 && Ready_Queue->PCB[1].burstTime < p->burstTime) {
        p->cpu_state = THE_CPU;
        Process tmp = *p;
        dequeueGeneric(Ready_Queue); // Remove current from heap
        enqueueBurst(tmp, Ready_Queue); // Re-insert with new burst time
        break;
      }
    }
    
    bool finished = (p->burstTime <= 0) || (THE_CPU.hw_registers[PC] == CPU_HALT);
    if (finished) {
      p->cpu_state = THE_CPU;
      printf("Process %d finished - freeing memory\n", p->pid);
      liberate(p->pid);
      dequeueGeneric(Ready_Queue);
    }
  }
  
  set_current_process(SYSTEM_PROCESS_ID);
}

static void highestResponseRatioNext(void) {
  transferProcesses(NORMAL);
  
  int total_time = 0;
  
  while (Ready_Queue->next > 0) {
    updateResponseRatio(Ready_Queue, total_time);
    
    int best_idx = getHighestResponseRatioIndex();
    if (best_idx < 0) break;
    
    // Swap best to front
    if (best_idx != 0) {
      swap(Ready_Queue, 0, best_idx);
    }
    
    Process *p = &Ready_Queue->PCB[0];
    set_current_process(p->pid);
    THE_CPU = p->cpu_state;
    
    int process_time = p->burstTime;
    
    while (p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT) {
      fetch();
      execute();
      p->burstTime--;
    }
    
    p->cpu_state = THE_CPU;
    total_time += process_time;
    
    printf("Process %d finished - freeing memory\n", p->pid);
    liberate(p->pid);
    shift_ready_left();
    
    transferProcesses(NORMAL);
  }
  
  set_current_process(SYSTEM_PROCESS_ID);
}

static void feedBack(void) {
  Queue* feedBack_Q2 = init_FeedBack_Queue(MAX_PROCESSES);
  Queue* feedBack_Q3 = init_FeedBack_Queue(MAX_PROCESSES);
  int quantum1 = 2;
  int quantum2 = 4;

  transferProcesses(NORMAL);

  while(Ready_Queue->next > 0 || feedBack_Q2->next > 0 || feedBack_Q3->next > 0) {
    transferProcesses(NORMAL);
    
    // Handle Q1 (highest priority) with quantum 2
    if (Ready_Queue->next > 0) {
      Process *p = &Ready_Queue->PCB[0];
      set_current_process(p->pid);
      THE_CPU = p->cpu_state;
      
      for (int i = 0; i < quantum1 && p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT; i++) {
        fetch();
        execute();
        p->burstTime--;
      }
      
      p->cpu_state = THE_CPU;
      
      bool finished = (p->burstTime <= 0) || (THE_CPU.hw_registers[PC] == CPU_HALT);
      if (finished) {
        printf("Process %d finished - freeing memory\n", p->pid);
        liberate(p->pid);
        shift_ready_left();
      } else {
        Process tmp = *p;
        shift_ready_left();
        enqueueGeneric(tmp, feedBack_Q2);
      }
      continue;
    }
    
    // Handle Q2 (middle priority) with quantum 4
    if (feedBack_Q2->next > 0) {
      Process *p = &feedBack_Q2->PCB[0];
      set_current_process(p->pid);
      THE_CPU = p->cpu_state;
      
      for (int i = 0; i < quantum2 && p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT; i++) {
        fetch();
        execute();
        p->burstTime--;
      }
      
      p->cpu_state = THE_CPU;
      
      bool finished = (p->burstTime <= 0) || (THE_CPU.hw_registers[PC] == CPU_HALT);
      if (finished) {
        printf("Process %d finished - freeing memory\n", p->pid);
        liberate(p->pid);
        dequeueGeneric(feedBack_Q2);
      } else {
        Process tmp = dequeueGeneric(feedBack_Q2);
        enqueueGeneric(tmp, feedBack_Q3);
      }
      continue;
    }
    
    // Handle Q3 (lowest priority) FCFS
    if (feedBack_Q3->next > 0) {
      Process *p = &feedBack_Q3->PCB[0];
      set_current_process(p->pid);
      THE_CPU = p->cpu_state;
      
      while (p->burstTime > 0 && THE_CPU.hw_registers[PC] != CPU_HALT) {
        fetch();
        execute();
        p->burstTime--;
      }
      
      p->cpu_state = THE_CPU;
      printf("Process %d finished - freeing memory\n", p->pid);
      liberate(p->pid);
      dequeueGeneric(feedBack_Q3);
    }
  }

  free_Queue(feedBack_Q2);
  free_Queue(feedBack_Q3);
  set_current_process(SYSTEM_PROCESS_ID);
}

//-------------------------------------Scheduler-------------------------------------//

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
