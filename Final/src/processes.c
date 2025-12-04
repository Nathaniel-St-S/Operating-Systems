#include "../include/processes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-------------------------------------Structs & Enums-------------------------------------//
typedef enum {
  RR,
  PRIORITY,
  SRT,
  HRRN,
  FCFS,
  SPN,
  FEEDBACK
} Scheduler;

typedef enum {
  NORMAL,
  PRIORITYBURST,
  PRIORITYPRIORITY
} QueueType;

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
    int pc; 
    int acc;
    ProcessState state;
    int priority; //priority level
    int burstTime; //time left to complete
    float responseRatio; //calculated as (waiting time + service time) //service time 
} Process;

//To represent a queue
typedef struct {
    int next; //the index to next open space
    int capacity; //The size of the queue
    Process PCB[]; //The block to hold processes
} Queue;

//-------------------------------------Constants-------------------------------------//
static Queue* Ready_Queue = NULL;
static Queue* Blocked_Queue = NULL;
static Queue* Running_Queue = NULL;
static Queue* Suspend_Ready_Queue = NULL;
static Queue* Suspend_Blocked_Queue = NULL;
static Queue* New_Queue = NULL;
static Queue* Finished_Queue = NULL;

static Process P1 = {0, 1, 10, READY, 3, 8, 3.2};
static Process P2 = {0, 2, 12, BLOCKED, 2, 8, 3.2};
static Process P3 = {0, 6, 11, RUNNING, 4, 8, 3.2};

//-------------------------------------Initializers for Queue-------------------------------------//

void init_Ready_Queue(const int size) {
    Queue* Ready_Queue = calloc(size, sizeof(Process));
    Ready_Queue->next = 0;
    Ready_Queue->capacity = size;
    Ready_Queue->PCB[size];
    if (!Ready_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Running_Queue(const int size) {
    Queue* Running_Queue = calloc(size, sizeof(Process));
    Running_Queue->next = 0;
    Running_Queue->capacity = size;
    Running_Queue->PCB[size];
    if (!Running_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Blocked_Queue(const int size) {
    Queue* Blocked_Queue = calloc(size, sizeof(Process));
    Blocked_Queue->next = 0;
    Blocked_Queue->capacity = size;
    Blocked_Queue->PCB[size];
    if (!Blocked_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Suspend_Block_Queue(const int size) {
    Queue* Suspend_Block_Queue = calloc(size, sizeof(Process));
    Suspend_Block_Queue->next = 0;
    Suspend_Block_Queue->capacity = size;
    Suspend_Block_Queue->PCB[size];
    if (!Suspend_Block_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Suspend_Ready_Queue(const int size) {
    Queue* Suspend_Ready_Queue = calloc(size, sizeof(Process));
    Suspend_Ready_Queue->next = 0;
    Suspend_Ready_Queue->capacity = size;
    Suspend_Ready_Queue->PCB[size];
    if (!Suspend_Ready_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_New_Queue(const int size) {
    Queue* New_Queue = calloc(size, sizeof(Process));
    New_Queue->next = 0;
    New_Queue->capacity = size;
    New_Queue->PCB[size];
    if (!New_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}
   
void init_Finished_Queue(const int size) {
    Queue* Finished_Queue = calloc(size, sizeof(Process));
    Finished_Queue->next = 0;
    Finished_Queue->capacity = size;
    Finished_Queue->PCB[size];
    if (!Finished_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Queues(const int size) {
    init_Ready_Queue(size);
    init_Running_Queue(size);
    init_Blocked_Queue(size);
    init_Suspend_Block_Queue(size);
    init_Suspend_Ready_Queue(size);
    init_New_Queue(size);
    init_Finished_Queue(size);
}

void free_Queue(Queue* Q) {
    free(Q);
}

//-------------------------------------Helpers for Queue-------------------------------------//

//for non priority queues
void enqueue(Process elem, Queue* Q) {
    if (Q->next >= Q->capacity) {
        fprintf(stderr, "Queue is full\n");
        return;
    }

    Q->PCB[Q->next] = elem;
    Q->next+=1;
}

//for non priority queues
Process dequeue(Queue* Q) {
    if (Q->next == 0) {
        fprintf(stderr, "Queue is empty\n");
        return;
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

int length(Queue* Q) {
    int i = 0;
    while (i < Q->next) {
        i+=1;
    }
    return i; 
}

void printProcess(Queue* Q) {
    int i = 0;
    while (i < Q->next) {
        printf("\nProcess %d \n", Q->PCB[i].burstTime);
        i+=1;
    }
}

// Define swap function to swap two PCB
void swap(Queue* Q, int swappee, int swapper) {
    Process temp = Q->PCB[swappee];
    Q->PCB[swappee] = Q->PCB[swapper];
    Q->PCB[swapper] = temp;
}

Process searchForShortestProcess() {
    int i = 0; 
    Process shortestP = Ready_Queue->PCB[i];
    while (i+1 < Ready_Queue->PCB) {
        if (Ready_Queue->PCB[i+1].burstTime < shortestP.burstTime) {
            shortestP = Ready_Queue->PCB[i+1];
        }
        i+=1;
    }

    return shortestP;
}

void removeProcess(Process P, Queue* Q) {
    int i = 0;
    while (i < Q->next &&  &P != &Q->PCB[i]) {
        i+=1;
    }
    swap(Q, 0, i);
    dequeue(Q);
}

// Adds a procces to the priority queue WRT to a process's burst time
void enqueue_Burst(Process process, Queue* Q) {
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

//extracts a process and maintains the priority queue WRT burst time
Process dequeueBurst(Queue* Q) {
    if (Q->next == 0) {
        fprintf(stderr, "Priority queue is empty\n");
        return;
    }

    Process process = Q->PCB[0];

    Q->PCB[0] = Q->PCB[--Q->next];
    int index = 0;
    while(1) {
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
void enqueuePriority(Process process, Queue* Q) {
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

// extracts a process and maintains the priority queue WRT priority
Process dequeuePriority(Queue* Q) {
    if (Q->next == 0) {
        fprintf(stderr, "Priority queue is empty\n");
        return;
    }

    Process process = Q->PCB[0];

    Q->PCB[0] = Q->PCB[--Q->next];
    int index = 0;
    while(1) {
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


void Enqueue(Process P) {
    switch (P.state) {
        case READY: enqueue(P, Ready_Queue); break;
        case BLOCKED: enqueue(P, Blocked_Queue); break;
        case SUSPEND_READY: enqueue(P, Suspend_Ready_Queue); break;
        case SUSPEND_BLOCKED: enqueue(P, Suspend_Blocked_Queue); break;
        case RUNNING: enqueue(P, Running_Queue); break;
        case NEW: enqueue(P, New_Queue); break;
        case FINISHED: enqueue(P, Finished_Queue); break;  
    }
}

Process Dequeue(Queue* Q) {
    return dequeue(Q);
}

//-------------------------------------State Transitions-------------------------------------//
void transitionState(Process P) {
    //stub
}

//-------------------------------------Context Switching-------------------------------------//

//Switches from one process to another
void context_switch(int current, int next) {
    Process* curr = &Ready_Queue->PCB[current];
    Process* nxt = &Ready_Queue->PCB[next];
  
    //save current's state
    //curr->cpu_state = THE_CPU;
    curr->state = READY;

    //start the next process
    //THE_CPU = nxt->cpu_state;
    nxt->state = RUNNING;

    printf("Switched from process (PID: %d) to process (PID: %d)", curr->pid, nxt->pid);
}

//-------------------------------------Scheduling Algorithms-------------------------------------//


//The first come first serve scheduling algorithm
void firstComeFirstServe(void) {
    while (Ready_Queue->next != 0) {
        Process currentProcess = dequeue(Ready_Queue);
        for (int i = 0; i < currentProcess.burstTime; i++) {
            fetch();
            execute();
        }
    }
}

void shortestProcessNext(void) {
    while (Ready_Queue->next != 0) {
        Process shortestProcess = searchForShortestProcess(Ready_Queue);
        removeProcess(shortestProcess, Ready_Queue);
        for (int i = 0; i < shortestProcess.burstTime; i++) {
            fetch();
            execute();
        }
    }
}