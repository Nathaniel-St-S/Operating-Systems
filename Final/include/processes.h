#ifndef PROCESSES_H
#define PROCESSES_H
#include "cpu.h"
#include <stdlib.h>

#define MAX_PROCESSES 10

//---------------Processes------------------

// Process States
typedef enum {
  NEW,
  READY,
  RUNNING,
  BLOCKED,
  TERMINATED,
} ProcessState;

// Process Control Block
typedef struct {
  uint32_t pid;
  ProcessState state;
  uint32_t hw_registers[HW_REG_COUNT];
  uint32_t gp_registers[GP_REG_COUNT];
  int priority;
  int time_remaining;
  float response_ratio;
} PCB;

extern int TIME_QUANTUM;
extern int CURR_NUM_PROCESSES;
extern PCB PROCESS_TABLE[MAX_PROCESSES];

void context_switch(PCB *current, PCB *next);

//----------------Queue-----------------
typedef struct QueueNode {
  PCB *data;
  struct QueueNode *next;
} QueueNode;

typedef struct Queue {
  QueueNode *head;
  QueueNode *tail;
} Queue;

extern Queue *THE_PROCESS_QUEUE;

int process_queue_is_empty();

//-----------------Schedulers-------------------
typedef struct {
  void (*init)(void);
  void (*add_new)(PCB *p);
  void (*on_exit)(PCB *p);
  void (*on_block)(PCB *p);
  void (*on_unblock)(PCB *p);
  void (*on_tick)(void);
} Scheduler;

extern Scheduler round_robin;

extern Scheduler fcfs;

extern Scheduler priority;

extern Scheduler shortest_time_remaining;

extern Scheduler highest_response_ratio_next;

extern Scheduler shortest_process_next;

extern Scheduler multilevel_feedback_queue;

#endif // !PROCESSES_H
