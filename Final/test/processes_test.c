//#include "../include/PCB.h"
//#include "../include/memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        printf("❌ ASSERTION FAILED: %s (line %d)\n", #expr, __LINE__); \
        return 0; \
    } \
} while(0)

#define ASSERT_EQ_INT(a, b) ASSERT_TRUE((a) == (b))

#define TEST(name) int name()
#define RUN_TEST(t) do { \
    if (t()) printf("Yippe!!  %s passed\n", #t); \
    else     printf("AWWE :(  %s failed\n", #t); \
} while(0)


//To represent a process
typedef struct {
    int pid; //process id
    int pc; 
    int acc;
    char state[10]; //state (READY, RUNNING, BLOCKED)
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

//Initalizes the ready queue
Queue* init_Ready_Queue(const int size) {
    Queue* Ready_Queue = calloc(size, sizeof(Process));
    Ready_Queue->next = 0;
    Ready_Queue->capacity = size;
    Ready_Queue->PCB[size];
    if (!Ready_Queue) {
        perror("calloc R_Queue");
        exit(EXIT_FAILURE);
    }

    return Ready_Queue;
}
    

void free_Ready_Queue(Queue* Ready_Queue) {
    free(Ready_Queue);
}

//for non priority queues
void enqueue(Process elem, Queue* Q) {
    Q->PCB[Q->next] = elem;
    Q->next+=1;
}

//for non priority queues
Process dequeue(Queue* Q) {
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
    if (!Q->capacity) {
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

TEST(Test_Enqueue) {
    Queue* Ready_Queue = init_Ready_Queue(5);

    Process P1 = {0, 1, 10, "Ready", 3, 2, 3.2};
    Process P2 = {0, 2, 12, "Blocked", 2, 9, 3.2};
    Process P3 = {0, 6, 11, "Running", 4, 8, 3.2};

    enqueue(P1, Ready_Queue);
    enqueue(P2, Ready_Queue);
    enqueue(P3, Ready_Queue);


    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 9);
    ASSERT_EQ_INT(Ready_Queue->PCB[2].burstTime, 8);

    free_Ready_Queue(Ready_Queue);
    return 1;
}

TEST(Test_Dequeue) {
    Queue* Ready_Queue = init_Ready_Queue(5);

    Process P1 = {0, 1, 10, "Ready", 3, 2, 3.2};
    Process P2 = {0, 2, 12, "Blocked", 2, 9, 3.2};
    Process P3 = {0, 6, 11, "Running", 4, 8, 3.2};

    enqueue(P1, Ready_Queue);
    enqueue(P2, Ready_Queue);
    enqueue(P3, Ready_Queue);

    dequeue(Ready_Queue);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 9);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 8);
    
    free_Ready_Queue(Ready_Queue);
}

TEST(Test_Swap) {
    Queue* Ready_Queue = init_Ready_Queue(5);

    Process P1 = {0, 1, 10, "Ready", 3, 2, 3.2};
    Process P2 = {0, 2, 12, "Blocked", 2, 9, 3.2};

    enqueue(P1, Ready_Queue);
    enqueue(P2, Ready_Queue);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 9);
    
    swap(Ready_Queue, 0, 1);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 9);
    swap(Ready_Queue, 0, 1);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 9);

    free_Ready_Queue(Ready_Queue);
}

TEST(Test_PriortyPriorityQ) {
    Queue* Ready_Queue = init_Ready_Queue(5);

    Process P1 = {0, 1, 10, "Ready", 3, 2, 3.2};
    Process P2 = {0, 2, 12, "Blocked", 2, 9, 3.2};
    Process P3 = {0, 6, 11, "Running", 4, 8, 3.2};
    Process P4 = {0,4,9,"Ready", 1, 1, 2.2};
    Process P5 = {0,0,0, "Blocked", 0, 0, 0};

    enqueuePriority(P1, Ready_Queue);
    enqueuePriority(P2, Ready_Queue);
    enqueuePriority(P3, Ready_Queue);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].priority, 3);
    ASSERT_EQ_INT(Ready_Queue->PCB[2].priority, 4);

    enqueuePriority(P4, Ready_Queue);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, 1);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].priority, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[2].priority, 4);
    ASSERT_EQ_INT(Ready_Queue->PCB[3].priority, 3);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, dequeuePriority(Ready_Queue).priority);
    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, dequeuePriority(Ready_Queue).priority);
    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, dequeuePriority(Ready_Queue).priority);
    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, dequeuePriority(Ready_Queue).priority);

    enqueuePriority(P5, Ready_Queue);
    ASSERT_EQ_INT(Ready_Queue->PCB[0].priority, 0);

    free_Ready_Queue(Ready_Queue);
}

TEST(Test_PriorityBurstQ) {
    Queue* Ready_Queue = init_Ready_Queue(5);

    Process P1 = {0, 1, 10, "Ready", 3, 2, 3.2};
    Process P2 = {0, 2, 12, "Blocked", 2, 9, 3.2};
    Process P3 = {0, 6, 11, "Running", 4, 8, 3.2};
    Process P4 = {0,4,9,"Ready", 1, 1, 2.2};
    Process P5 = {0,0,0, "Blocked", 0, 0, 0};
    Process P6 = {0,0,0, "Blocked", 0, 0, 0};

    enqueue_Burst(P1, Ready_Queue);
    enqueue_Burst(P2, Ready_Queue);  
    enqueue_Burst(P3, Ready_Queue);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 9);
    ASSERT_EQ_INT(Ready_Queue->PCB[2].burstTime, 8);

    enqueue_Burst(P4, Ready_Queue);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 1);
    ASSERT_EQ_INT(Ready_Queue->PCB[1].burstTime, 2);
    ASSERT_EQ_INT(Ready_Queue->PCB[2].burstTime, 8);
    ASSERT_EQ_INT(Ready_Queue->PCB[3].burstTime, 9);

    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, dequeueBurst(Ready_Queue).burstTime); //1
    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, dequeueBurst(Ready_Queue).burstTime); //2
    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, dequeueBurst(Ready_Queue).burstTime); //8
    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, dequeueBurst(Ready_Queue).burstTime); //9

    enqueue_Burst(P5, Ready_Queue);
    ASSERT_EQ_INT(Ready_Queue->PCB[0].burstTime, 0); //0


    printf("P5 %p \nP6 %p\n", &P5, &P6);
    printf("same process %p %d\n", &P5, &P5 == &P5);
    free_Ready_Queue(Ready_Queue);
}

int main() {
    RUN_TEST(Test_Enqueue);
    RUN_TEST(Test_Dequeue);
    RUN_TEST(Test_Swap);
    RUN_TEST(Test_PriorityBurstQ);
    RUN_TEST(Test_PriortyPriorityQ);
    return 0;
}