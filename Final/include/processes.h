#ifndef PROCESSES_H
#define PROCESSES_H

#include <stdint.h>


#define QUANTUM 3;

typedef struct PCB {
    int pid; //process id
    int pc; 
    int acc;
    char state[10]; //state (READY, RUNNING, BLOCKED)
    int priority; //priority level
    int timeRemaining; //time left to complete
    float responseRatio; //calculated as (waiting time + service time) //service time 
} PCB;

typedef struct List {
    PCB* first;
    struct List* rest;
} List;

void init_Ready_Queue(const int size);

List* Cons(PCB* elem, List* L); 

int isEmpty(List* L); 

void Append(PCB elem, List* L);

void Enqueue(PCB elem, List* L);

List* Dequeue(List* L);

List* sortWRTBurstTime(List* L);

//ready_List = NULL;


//Process scheduling algorithms

void roundRobin(void);

void priorityBased(void);

void shortestRemainingTime(void);

void highestResponseRatioNext(void);

void firstComeFirstServe(void);

void shortProcessNext(void);

void feedback(void);

void context_switch(int current, int next);

#endif