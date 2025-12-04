#ifndef PROCESSES_H
#define PROCESSES_H

#define QUANTUM 3;

void roundRobin(void);

void priorityBased(void);

void shortestRemainingTime(void);

void highestResponseRatioNext(void);

void firstComeFirstServe(void);

void shortestProcessNext(void);

void feedback(void);

void context_switch(int current, int next);

#endif