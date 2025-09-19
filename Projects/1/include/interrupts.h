#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

#define MAX_INTERRUPTS 128

//#define INTERRUPTFLAG 1 //flag to signal interrupt

typedef enum irq{
    SAY_HI,
    SAY_GOODBYE,
    BREAK,
    EOI, //end of interupt, make sure this is the last in the list
}IRQ;

typedef struct {
    Cpu* items;
    int SP;
} stack;

typedef struct {
    IRQ irq;
    int priority;
} Interrupt;

typedef struct {
    Interrupt data[MAX_INTERRUPTS];
    int size;
} InterruptHeap;

int parent(int i) { return (i - 1) / 2; }
int left(int i)   { return 2 * i + 1; }
int right(int i)  { return 2 * i + 2; }

void swap(Interrupt *a, Interrupt *b) {
    Interrupt tmp = *a;
    *a = *b;
    *b = tmp;
}

void heap_insert(InterruptHeap *h, IRQ irq, int priority) {
    if (h->size >= MAX_INTERRUPTS) {
        printf("Heap overflow!\n");
        return;
    }

    // Insert new interrupt at end
    int i = h->size++;
    h->data[i].irq = irq;
    h->data[i].priority = priority;

    // Bubble up
    while (i != 0 && h->data[parent(i)].priority > h->data[i].priority) {
        swap(&h->data[i], &h->data[parent(i)]);
        i = parent(i);
    }
}

Interrupt heap_extract_min(InterruptHeap *h) {
    if (h->size <= 0) {
        printf("Heap underflow!\n");
        return (Interrupt){-1, -1};
    }

    Interrupt root = h->data[0];
    h->data[0] = h->data[--h->size];

    // Heapify down
    int i = 0;
    while (1) {
        int l = left(i), r = right(i), smallest = i;

        if (l < h->size && h->data[l].priority < h->data[smallest].priority)
            smallest = l;
        if (r < h->size && h->data[r].priority < h->data[smallest].priority)
            smallest = r;

        if (smallest != i) {
            swap(&h->data[i], &h->data[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return root;
}

//interrupt handler
void interrupt_handler(Interrupt intrpt);

//Checks for if any interrupts are present
void check_for_interrupt(void);

#endif 