#include "../include/processes.h"
#include "../include/cpu.h"
#include "../include/memory.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//-------------------------------------Structs & Enums-------------------------------------//
typedef enum {
  RR,
  PRIORITY,
  SRT,
  HRRN,
  FCFS,
  SPN,
  FEEDBACK
} SchedulerType;

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
static const int QUANTUM = 3;
static const int MAX_PROCESSES = 10;

static Queue* Ready_Queue = NULL;
static Queue* Blocked_Queue = NULL;
static Queue* Running_Queue = NULL;
static Queue* Suspend_Ready_Queue = NULL;
static Queue* Suspend_Blocked_Queue = NULL;
static Queue* New_Queue = NULL;
static Queue* Finished_Queue = NULL;

//-------------------------------------Initializers for Queue-------------------------------------//

void init_Ready_Queue(const int size) {
    Queue* Ready_Queue = calloc(size, sizeof(Process));
    Ready_Queue->next = 0;
    Ready_Queue->capacity = size;
    Ready_Queue->PCB[size];
    if (!Ready_Queue) {
        perror("calloc Ready_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Running_Queue(const int size) {
    Queue* Running_Queue = calloc(size, sizeof(Process));
    Running_Queue->next = 0;
    Running_Queue->capacity = size;
    Running_Queue->PCB[size];
    if (!Running_Queue) {
        perror("calloc Running_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Blocked_Queue(const int size) {
    Queue* Blocked_Queue = calloc(size, sizeof(Process));
    Blocked_Queue->next = 0;
    Blocked_Queue->capacity = size;
    Blocked_Queue->PCB[size];
    if (!Blocked_Queue) {
        perror("calloc Blocked_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Suspend_Block_Queue(const int size) {
    Queue* Suspend_Block_Queue = calloc(size, sizeof(Process));
    Suspend_Block_Queue->next = 0;
    Suspend_Block_Queue->capacity = size;
    Suspend_Block_Queue->PCB[size];
    if (!Suspend_Block_Queue) {
        perror("calloc Suspend_Block_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_Suspend_Ready_Queue(const int size) {
    Queue* Suspend_Ready_Queue = calloc(size, sizeof(Process));
    Suspend_Ready_Queue->next = 0;
    Suspend_Ready_Queue->capacity = size;
    Suspend_Ready_Queue->PCB[size];
    if (!Suspend_Ready_Queue) {
        perror("calloc Suspend_Ready_Queue");
        exit(EXIT_FAILURE);
    }
}

void init_New_Queue(const int size) {
    Queue* New_Queue = calloc(size, sizeof(Process));
    New_Queue->next = 0;
    New_Queue->capacity = size;
    New_Queue->PCB[size];
    if (!New_Queue) {
        perror("calloc New_Queue");
        exit(EXIT_FAILURE);
    }
}
   
void init_Finished_Queue(const int size) {
    Queue* Finished_Queue = calloc(size, sizeof(Process));
    Finished_Queue->next = 0;
    Finished_Queue->capacity = size;
    Finished_Queue->PCB[size];
    if (!Finished_Queue) {
        perror("calloc Finished_Queue");
        exit(EXIT_FAILURE);
    }
}

Queue* init_FeedBack_Queue(const int size) {
    Queue* FeedBack_Queue = calloc(size, sizeof(Process));
    FeedBack_Queue->next = 0;
    FeedBack_Queue->capacity = size;
    FeedBack_Queue->PCB[size];
    if (!FeedBack_Queue) {
        perror("calloc FeedBack_Queue");
        exit(EXIT_FAILURE);
    }

    return FeedBack_Queue;
}

void free_Queue(Queue* Q) {
    free(Q);
}

//-------------------------------------Helpers for Queue-------------------------------------//

//for non priority queues
void enqueue(Process elem, Queue* Q) {
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

// Define swap function to swap two PCB
void swap(Queue* Q, int swappee, int swapper) {
    Process temp = Q->PCB[swappee];
    Q->PCB[swappee] = Q->PCB[swapper];
    Q->PCB[swapper] = temp;
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

//enqueues the process depending on the queue type
void EnqueueHelper(Process P, int Queue_Type) {
    switch(Queue_Type) {
        case NORMAL: enqueue(P, Ready_Queue); break;
        case PRIORITYBURST: enqueue_Burst(P, Ready_Queue); break;
        case PRIORITYPRIORITY: enqueuePriority(P, Ready_Queue); break;
        default: fprintf(stderr, "Unknown Queue Type\n"); break;
    }
}

//enqueues the process depending the process's state
void Enqueue(Process P, int Queue_Type) {
    switch (P.state) {
        case READY: EnqueueHelper(P, Queue_Type); break; 
        case BLOCKED: enqueue(P, Blocked_Queue); break;
        case SUSPEND_READY: enqueue(P, Suspend_Ready_Queue); break;
        case SUSPEND_BLOCKED: enqueue(P, Suspend_Blocked_Queue); break;
        case RUNNING: enqueue(P, Running_Queue); break;
        case NEW: enqueue(P, New_Queue); break;
        case FINISHED: enqueue(P, Finished_Queue); break; 
        default: fprintf(stderr, "Unknown Process State\n"); break;  
    }
}

//Returns the process at the front of the queue depending on the queue type
Process Dequeue(Queue* Q, int Queue_Type) {
    Process P;
    switch(Queue_Type) {
        case NORMAL: P = dequeue(Q); break;
        case PRIORITYBURST: P = dequeueBurst(Q); break;
        case PRIORITYPRIORITY: P = dequeuePriority(Q); break; 
    } 
    return P;
}

//-------------------------------------State Transitions-------------------------------------//

//transitions a process 's state from one to another and moves to its corresponding queue
void transitionState(Process P) {
    if (P.burstTime <= 0) {
        P.state = FINISHED;
    } else if ((P.state == NEW) || (P.burstTime > 0 && RUNNING)) {
        P.state = READY;
    } else {
        P.state = RUNNING;
    }
}

//-------------------------------------Process Creation-------------------------------------//

Process* makeProcess(int pID, int pc, int priority, int burstTime) {
    Process newProcess = {pID, pc, NEW, priority, burstTime, 0, THE_CPU};
    uint32_t processBlock = mallocate(pID, MAX_PROCESS_SIZE);
    Enqueue(newProcess, New_Queue);
    return &newProcess;
}

//-------------------------------------Context Switching-------------------------------------//

//Switches from one process to another
void context_switch(Queue* Q, int current, int next) {
    Process* curr = &Q->PCB[current];
    Process* nxt = &Q->PCB[next];
  
    //save current's state
    curr->cpu_state = THE_CPU;
    curr->state = READY;

    //start the next process
    THE_CPU = nxt->cpu_state;
    nxt->state = RUNNING;

    printf("Switched from process (PID: %d) to process (PID: %d)", curr->pid, nxt->pid);
}

//-------------------------------------Scheduling Helpers-------------------------------------//

//returns the process with the highest response ratio
Process getHighestResponseRatio() {
    int i = 1; 
    Process HRRProcess = Ready_Queue->PCB[0];
    while (i < Ready_Queue->next) {
        if (Ready_Queue->PCB[i].responseRatio < HRRProcess.responseRatio) {
            HRRProcess = Ready_Queue->PCB[i];
        }
        i+=1;
    }

    return HRRProcess;
}

//removes the given process from the given queue
void removeProcess(Process P, Queue* Q) {
    int i = 0;
    while (i < Q->next &&  &P != &Q->PCB[i]) {
        i+=1;
    }
    swap(Q, 0, i);
    Enqueue(Dequeue(Q, NORMAL), NORMAL);
}

//calculates given process's response ratio
int calcResponseRatio(Process P, int idleTime) {
    return (idleTime + P.burstTime) / P.burstTime;
}

//updates the response ratio for every process in the given queue
void updateResponseRatio(Queue* Q, int idleTime) {
    int i = 0;
    while (i < Q->next) {
        Q->PCB[i].responseRatio = calcResponseRatio(Q->PCB[i], idleTime);
        i++;
    }
}

//-------------------------------------Scheduling Algorithms-------------------------------------//

//the round robin scheduling algorithm
void roundRobin(void) {
    int idx = 0;
    while (Ready_Queue->next != 0) {
        Process currentProcess = Ready_Queue->PCB[idx];
        transitionState(currentProcess);
        //currentProcess.state = RUNNING;
        for (int i = 0; i < QUANTUM; i++) {
            fetch();
            execute();
            currentProcess.burstTime=-1; //could make ternary
        }

        if (currentProcess.burstTime > 0 && idx+1 >= Ready_Queue->next) {
            context_switch(Ready_Queue, idx, 0);
            idx = 0;
        } else if (currentProcess.burstTime > 0) {
            context_switch(Ready_Queue, idx, idx+1);
            idx+=1;
        } else {
            transitionState(currentProcess);
            Enqueue(Dequeue(Ready_Queue, NORMAL), NORMAL);
        }
         
    }
}


//The first come first serve scheduling algorithm
void firstComeFirstServe(void) {
    while (Ready_Queue->next != 0) {
        Process currentProcess = Ready_Queue->PCB[0];
        transitionState(currentProcess);
        for (int i = 0; i < currentProcess.burstTime; i++) {
            fetch();
            execute();
        }
        transitionState(currentProcess);
        Enqueue(Dequeue(Ready_Queue, NORMAL), NORMAL);
    }
}

//uses priorityBurstQueue 
//the shortest process next scheduling algorithm
void shortestProcessNext(void) {
    while (Ready_Queue->next != 0) {
        Process shortestProcess = Ready_Queue->PCB[0]; //searchForShortestProcess(Ready_Queue);
        transitionState(shortestProcess);
        for (int i = 0; i < shortestProcess.burstTime; i++) {
            fetch();
            execute();
        }
        transitionState(shortestProcess); 
        Enqueue(Dequeue(Ready_Queue, PRIORITYBURST), PRIORITYBURST);
        //removeProcess(shortestProcess, Ready_Queue);
    }
}

//uses priorityPriorityQueue 
//the priority based scheduling algorithm
void priorityBased(void) {
    while (Ready_Queue->next != 0) {
        Process highestPriorityP = Ready_Queue->PCB[0];
        transitionState(highestPriorityP);
        for (int i = 0; i < highestPriorityP.burstTime; i++) {
            fetch();
            execute();
            Process newHighestP = Ready_Queue->PCB[0]; //searchForHighestPriority(Ready_Queue);
            if (&newHighestP != &highestPriorityP) {
                if (&Ready_Queue->PCB[1] == &newHighestP) {
                    context_switch(Ready_Queue, 0, 1);
                } else {
                    context_switch(Ready_Queue, 0, 2);
                }
            }
        }
        transitionState(highestPriorityP);
        Enqueue(Dequeue(Ready_Queue, PRIORITYPRIORITY), PRIORITYPRIORITY);
    }
}
//uses priorityBurstQueue 
//the shortest time remaining scheduling algorithm
void shortestRemainingTime(void) {
    while (Ready_Queue->next != 0) {
        Process shortestBTimeP = Ready_Queue->PCB[0];
        transitionState(shortestBTimeP);
        for (int i = 0; i < shortestBTimeP.burstTime; i++) {
            fetch();
            execute();
            Process newShortestBTimeP = Ready_Queue->PCB[0]; //searchForShortestProcess(Ready_Queue);
            if (&newShortestBTimeP != &shortestBTimeP) {
                if (&Ready_Queue->PCB[1] == &newShortestBTimeP) {
                    context_switch(Ready_Queue, 0, 1);
                } else {
                    context_switch(Ready_Queue, 0, 2);
                }
            }
        }
        transitionState(shortestBTimeP);
        Enqueue(Dequeue(Ready_Queue, PRIORITYBURST), PRIORITYBURST);
    }
}

//the highest response ratio next scheduling algorithm
void highestResponseRatioNext(void) {
    if (Ready_Queue->next != 0 ) {
        int total_time = 0;
        Process currentProcess = Ready_Queue->PCB[0];
        total_time = currentProcess.burstTime;
        
        while (Ready_Queue->next != 0); {
            for (int i = 0; i < currentProcess.burstTime; i++) {
                fetch();
                execute();
            }
            transitionState(currentProcess);
            removeProcess(currentProcess, Ready_Queue);
            updateResponseRatio(Ready_Queue, total_time); 
            currentProcess = getHighestResponseRatio();
            total_time = currentProcess.burstTime;
            transitionState(currentProcess);
        }
    }
}

//the feedback scheduling algorithm
void feedBack(void) {
    Queue* feedBack_Q2 = init_FeedBack_Queue(MAX_PROCESSES);
    Queue* feedBack_Q3 = init_FeedBack_Queue(MAX_PROCESSES);
    int quantum1 = 2;
    int quantum2 = 4;

    while(true) {
        //handle processes in highest priority queue with 2 quantum
        if (Ready_Queue->next != 0) {
            Process P = Ready_Queue->PCB[0];
            transitionState(P);
            for (int i = 0; i < quantum1; i++) {
                fetch();
                execute();
                P.burstTime-=1;
            }
        }

        //handle processes in middle priority queue with 4 quantum
        if (feedBack_Q2->next != 0) {
            Process P = feedBack_Q2->PCB[0];
            transitionState(P);
            for (int j = 0; j < quantum2; j++) {
                fetch();
                execute();
                P.burstTime-=1;
            }
        }

        //handle processes in lowest priority queue FCFS
        if (feedBack_Q3->next != 0) {
            Process P = feedBack_Q3->PCB[0];
            transitionState(P);
            for (int k = 0; k < P.burstTime; k++) {
                fetch();
                execute();
            }
        }
        
        //highest priority queue not empty
        if (Ready_Queue->next != 0) {
            //move process from Ready queue to finished queue
            if (Ready_Queue->PCB[0].burstTime <= 0) {
                transitionState(Ready_Queue->PCB[0]);
                Enqueue(Dequeue(Ready_Queue, NORMAL), NORMAL);
            } else if (Ready_Queue->next >= 2) { // switch context then move to middle priority queue
                context_switch(Ready_Queue, 0, 1);
                enqueue(Dequeue(Ready_Queue, NORMAL), feedBack_Q2);
            } else { // move to middle priority queue
                enqueue(Dequeue(Ready_Queue, NORMAL), feedBack_Q2);
            }
        }

        //middle priority queue not empty
        if (feedBack_Q2->next != 0) {
            //move process from middle priority queue to finished queue
            if (feedBack_Q2->PCB[0].burstTime <= 0) {
                transitionState(feedBack_Q2->PCB[0]);
                Enqueue(Dequeue(feedBack_Q2, NORMAL), NORMAL);
            } else if (feedBack_Q2->next >= 2) { // switch context then move to lowest priority queue
                context_switch(feedBack_Q2, 0, 1);
                enqueue(Dequeue(feedBack_Q2, NORMAL), feedBack_Q3);
            } else { // move to lowest priority queue
                enqueue(Dequeue(feedBack_Q2, NORMAL), feedBack_Q3);
            }
        }

        //lowest priority queue not empty
        if (feedBack_Q3->next != 0) {
            //move process from middle priority queue to finished queue
            if (feedBack_Q3->PCB[0].burstTime <= 0) {
                transitionState(feedBack_Q3->PCB[0]);
                Enqueue(Dequeue(feedBack_Q3, NORMAL), NORMAL);
            }
        }
        
        if (Ready_Queue->next == 0 && feedBack_Q2->next == 0 && feedBack_Q3->next == 0) {
            break;
        }
    }
}

//-------------------------------------Scheduler-------------------------------------//

//to initializer the scheduler and queues using the given scheduler type
void initScheduler(int SchedulerType) {
    
    init_Ready_Queue(MAX_PROCESSES);
    init_Running_Queue(MAX_PROCESSES);
    init_Blocked_Queue(MAX_PROCESSES);
    init_Suspend_Block_Queue(MAX_PROCESSES);
    init_Suspend_Ready_Queue(MAX_PROCESSES);
    init_New_Queue(MAX_PROCESSES);
    init_Finished_Queue(MAX_PROCESSES);

    switch (SchedulerType) {
        case RR: roundRobin(); break;
        case PRIORITY: priorityBased(); break;
        case SRT: shortestRemainingTime(); break;
        case HRRN: highestResponseRatioNext(); break;
        case FCFS: firstComeFirstServe(); break;
        case SPN: shortestProcessNext(); break;
        case FEEDBACK: feedBack(); break;
        default: fprintf(stderr, "Unknown Scheduler Type\n"); break; 
    }
}