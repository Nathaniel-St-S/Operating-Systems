#include "../include/processes.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static List* Ready_Queue = NULL;

static PCB P1 = {0, 1, 10, "Ready", 3, 8, 3.2};
static PCB P2 = {0, 2, 12, "Blocked", 2, 8, 3.2};
static PCB P3 = {0, 6, 11, "Running", 4, 8, 3.2};

void init_Ready_Queue(const int size) {
    Ready_Queue = calloc(size, sizeof(PCB));
    Ready_Queue->rest = NULL;
    if (!Ready_Queue) {
        perror("calloc Ready_Queue");
        exit(EXIT_FAILURE);
    }
}

List* Cons(PCB* elem, List* L) {
    List* newList;
    newList = calloc(1, sizeof(elem));
    if (!newList) {
        perror("calloc  cons");
        exit(EXIT_FAILURE);
    }
    newList->first = elem;
    newList->rest = L; 
    return newList;
}

int length(List* L) {
    if (L->rest == NULL) {
        return 0;
    } else {
        return 1 + length(L->rest);
    }
}

int isEmpty(List* L) {
   return L == NULL;
}

//Adds the PCB to back of the given List
void Append(PCB elem, List* L) {
    if (isEmpty(L)) {
        List* newList;
        newList = calloc(1, sizeof(elem));
        if (!newList) {
            perror("calloc  cons");
            exit(EXIT_FAILURE);
        }
        newList->first = &elem;
        newList->rest = NULL;
        L->rest = newList;
    } else {
        Append(elem, L->rest);
    }
}

void Enqueue(PCB elem, List* L) {
    Append(elem, L);
}

List* Dequeue(List* L) {
    return L->rest; 
}

//determines if P1 has a smaller Burst time than P2
int hasSmallerBurstTime(PCB* P1, PCB* P2) {
    return P1->timeRemaining <= P2->timeRemaining; 
}

List* insertWRTBurst(PCB* elem, List* L) {
    if (isEmpty(L)) {
        return Cons(elem, L);
    } else if (hasSmallerBurstTime(elem, L->first)) {
        return Cons(elem, L);
    } else {
        L->rest = insertWRTBurst(elem, L->rest);
        return L;
    }
}

List* sortWRTBurstTime(List* L) {
    if (isEmpty(L)) {
    return L;
    } else {
    return insertWRTBurst(L->first, sortWRTBurstTime(L->rest));
    }
}

//determines if P1 has a smaller priority than P2
int hasSmallerPriority(PCB* P1, PCB* P2) {
    return P1->timeRemaining <= P2->timeRemaining; 
}

List* insertWRTPriority(PCB* elem, List* L) {
    if (isEmpty(L)) {
        return Cons(elem, L);
    } else if (hasSmallerPriority(elem, L->first)) {
        return Cons(elem, L);
    } else {
        L->rest = insertWRTPriority(elem, L->rest);
        return L;
    }
}

List* sortWRTPriority(List* L) {
   if (isEmpty(L)) {
    return L;
    } else {
    return insertWRTPriority(L->first, sortWRTPriority(L->rest));
    }
}


void main() {
    init_Ready_Queue(5);
    printf("done");
    
    int isIt = isEmpty(Ready_Queue);
    printf("\n first: %p\n", Ready_Queue->first);
    printf("\nlength: %d\n", length(Ready_Queue));
    printf("\n result: %d\n", isIt);
    Ready_Queue = Cons(&P1, Ready_Queue);
    printf("\n first: %p\n", Ready_Queue->first);
    printf("\nlength: %d\n", length(Ready_Queue));
    isIt = isEmpty(Ready_Queue);
    printf("\n result: %d\n", isIt);
    Ready_Queue = Cons(&P2, Ready_Queue);
    Ready_Queue = Cons(&P3, Ready_Queue);
    printf("\nlength: %d\n", length(Ready_Queue));
    isIt = isEmpty(Ready_Queue);
    printf("\n result: %d\n", isIt); 
    

   /* Append(P2, Ready_Queue);
    Append(P1, Ready_Queue);
    Append(P3, Ready_Queue);
    sortWRTBurstTime(Ready_Queue); */
} 