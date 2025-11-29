#include "../include/processes.h"
#include <stdint.h>
#include <stdlib.h>

//--------------Context Switching-------------------
static void save_process_context(PCB *pcb) {
    if (!pcb) return;

    for (int i = 0; i < GP_REG_COUNT; i++) {
        pcb->gp_registers[i] = THE_CPU.gp_registers[i];
    }

    for (int i = 0; i < HW_REG_COUNT; i++) {
        pcb->hw_registers[i] = THE_CPU.hw_registers[i];
    }
}

static void restore_process_context(const PCB *pcb) {
    if (!pcb) return;

    for (int i = 0; i < GP_REG_COUNT; i++) {
        THE_CPU.gp_registers[i] = pcb->gp_registers[i];
    }

    for (int i = 0; i < HW_REG_COUNT; i++) {
        THE_CPU.hw_registers[i] = pcb->hw_registers[i];
    }
}

void context_switch(PCB *current, PCB *next) {
    // If it's the same process or there's no target, nothing next do
    if (current == next || next == NULL) {
        return;
    }

    // 1. Save the currently running process
    if (current != NULL) {
        save_process_context(current);
        if (current->state == RUNNING) {
            current->state = READY;
        }
    }

    // 2. Resnextre the next process's context
    restore_process_context(next);
    next->state = RUNNING;
}

//----------------Process Queue-----------------
static Queue _process_queue = { .head = NULL, .tail = NULL };
Queue *THE_PROCESS_QUEUE = &_process_queue;

int process_queue_is_empty() {
  return (THE_PROCESS_QUEUE->head == NULL);
}

void process_enqueue(PCB *process) {
    if (!process) return;

    QueueNode *new_node = malloc(sizeof(QueueNode));
    if (!new_node) {
        // TODO: Probably fail here
        return;
    }

    new_node->data = process;
    new_node->next = NULL;

    if (process_queue_is_empty()) {
        THE_PROCESS_QUEUE->head = new_node;
        THE_PROCESS_QUEUE->tail = new_node;
    } else {
        THE_PROCESS_QUEUE->tail->next = new_node;
        THE_PROCESS_QUEUE->tail = new_node;
    }
}

PCB *process_dequeue(void) {
    if (process_queue_is_empty()) {
        return NULL;
    }

    QueueNode *node = THE_PROCESS_QUEUE->head;
    PCB *proc = node->data;

    THE_PROCESS_QUEUE->head = node->next;

    if (THE_PROCESS_QUEUE->head == NULL) {
        THE_PROCESS_QUEUE->tail = NULL;
    }

    free(node);
    return proc;
}

//-----------------Schedulers-------------------

PCB *CURRENT_PROCESS = NULL;

// Round Robin
static uint32_t quantum_remaining = 0;
int TIME_QUANTUM = 3;

void rr_init(void) {
    THE_PROCESS_QUEUE->head = NULL;
    THE_PROCESS_QUEUE->tail = NULL;

    CURRENT_PROCESS = NULL;
    quantum_remaining = 0;
}

void rr_add_process(PCB *p) {
    if (!p) return;

    p->state = READY;
    process_enqueue(p);
}

static PCB *pick_next_process(void) {
    PCB *next = process_dequeue();
    if (next) {
        next->state = RUNNING;
    }
    return next;
}

static void rotate(PCB *next) {
    if (CURRENT_PROCESS == next) {
        return;
    }

    // Switch CPU context
    context_switch(CURRENT_PROCESS, next);

    CURRENT_PROCESS = next;
    quantum_remaining = TIME_QUANTUM;
}

void rr_on_exit(PCB *p) {
    if (!p) return;

    p->state = TERMINATED;

    if (p == CURRENT_PROCESS) {
        PCB *next = pick_next_process();
        if (next) {
            rotate(next);
        } else {
            // No processes left to run
            CURRENT_PROCESS = NULL;
            quantum_remaining = 0;
        }
    }
}

void rr_on_block(PCB *p) {
    if (!p) return;

    p->state = BLOCKED;

    if (p == CURRENT_PROCESS) {
        PCB *next = pick_next_process();
        if (next) {
            rotate(next);
        } else {
            CURRENT_PROCESS = NULL;
            quantum_remaining = 0;
        }
    }
}

void rr_on_unblock(PCB *p) {
    if (!p) return;

    p->state = READY;
    process_enqueue(p);
}

void rr_on_tick(void) {
    // No running process? Try to start one.
    if (CURRENT_PROCESS == NULL) {
        PCB *next = pick_next_process();
        if (next) {
            rotate(next);
        }
        return;
    }

    // Decrement remaining quantum
    if (quantum_remaining > 0) {
        quantum_remaining--;
    }

    // If quantum not expired yet, just keep running
    if (quantum_remaining > 0) {
        return;
    }

    // Quantum expired: preempt CURRENT_PROCESS if it's still RUNNING
    PCB *old = CURRENT_PROCESS;

    if (old && old->state == RUNNING) {
        old->state = READY;
        process_enqueue(old);
    }

    PCB *next = pick_next_process();

    if (next) {
        rotate(next);
    } else {
        // No one else is ready; let old keep the CPU if it exists
        if (old && old->state == READY) {
            old->state = RUNNING;
            CURRENT_PROCESS = old;
            quantum_remaining = TIME_QUANTUM;
        } else {
            CURRENT_PROCESS = NULL;
            quantum_remaining = 0;
        }
    }
}

Scheduler round_robin = {
  .init = rr_init,
  .add_new = rr_add_process,
  .on_exit = rr_on_exit,
  .on_block = rr_on_block,
  .on_unblock = rr_on_unblock,
  .on_tick = rr_on_tick
};

Scheduler fcfs = { };

Scheduler priority = { };

Scheduler shortest_time_remaining = { };

Scheduler highest_response_ratio_next = { };

Scheduler shortest_process_next = { };

Scheduler multilevel_feedback_queue = { };
