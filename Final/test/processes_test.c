#include "../include/processes.h"
//#include "framework.h"

#include <stdio.h>
#include <stdlib.h>
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
    if (t()) printf("✔️  %s passed\n", #t); \
    else     printf("❌ %s failed\n", #t); \
} while(0)

int listToArray(List* L, PCB* out[], int max) {
    int i = 0;
    while (!isEmpty(L) && i < max) {
        out[i++] = L->first;
        L = L->rest;
    }
    return i;
}

PCB* initPCB(int pid, int pc, int acc, char state[8], int priority, int burstTime, float responseRatio) {
    PCB* process = calloc(1, sizeof(PCB));
    process->pid = pid;
    process->pc = pc;
    process->acc = acc;
    process->state[8] = state;
    process->priority = priority;
    process->timeRemaining = burstTime;
    process->responseRatio = responseRatio;
    return process;
}

TEST(test_sort_burst_time) {
    static List* Ready_Queue = NULL;

    // Create three PCBs with different burst times
    PCB* P1 = initPCB(0, 1, 10, "Ready", 3, 2, 3.2);
    PCB* P2 = initPCB(0, 2, 12, "Blocked", 2, 9, 3.2);
    PCB* P3 = initPCB(0, 6, 11, "Running", 4, 8, 3.2);

    init_Ready_Queue(5);

    // Build an unsorted list: [p1, p2, p3]
    List* L = /*Ready_Queue;*/ Cons(P1, Cons(P2, Cons(P3, Ready_Queue)));

    // Sort it
    List* sorted = sortWRTBurstTime(L);

    // Convert to array for checking
    PCB* arr[3];
    int n = listToArray(sorted, arr, 3);

    ASSERT_EQ_INT(n, 3);

    // Verify sorted by burst time: 3, 5, 9
    ASSERT_EQ_INT(arr[0]->timeRemaining, 3);
    ASSERT_EQ_INT(arr[1]->timeRemaining, 5);
    ASSERT_EQ_INT(arr[2]->timeRemaining, 9);

    return 1;
}

int main() {
    //printf("hlello a");
    RUN_TEST(test_sort_burst_time);
    printf("done");
    return 0;
}