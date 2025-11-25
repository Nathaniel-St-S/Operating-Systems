#ifndef PROCESS_H
#define PROCESS_H

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
    PCB first;
    struct List* rest;
} List;

List cons(PCB elem, List L); 

int isEmpty(); 

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