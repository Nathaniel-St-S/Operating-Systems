#include "os.h"
#include <time.h>

int perf_contextSwitches = 0;
int perf_cpuActive = 0;
int perf_cpuIdle = 0;

static clock_t t_start = 0;

void perf_resetCounters(void) {
    perf_contextSwitches = 0;
    perf_cpuActive = 0;
    perf_cpuIdle = 0;
}

void perf_beginTimer(void) {
    t_start = clock();
}

void perf_endTimer(Metrics *m) {
    clock_t t_end = clock();
    m->execTime = (double)(t_end - t_start) / CLOCKS_PER_SEC;
}

/* Compute waiting, turnaround, CPU util */
void perf_finalizeMetrics(Metrics *m) {
    if (completedCount == 0) {
        m->avgWaiting = 0;
        m->avgTurnaround = 0;
        m->cpuUtilization = 0;
        return;
    }

    int totalT = 0;
    int totalW = 0;
    int lastFinish = 0;
    int totalBurst = 0;

    for (int i = 0; i < completedCount; i++) {
        PCB p = completedTable[i];
        int turnaround = p.finishTime - p.arrivalTime;
        int waiting = turnaround - p.originalBurst;

        totalT += turnaround;
        totalW += waiting;
        totalBurst += p.originalBurst;

        if (p.finishTime > lastFinish)
            lastFinish = p.finishTime;
    }

    m->avgTurnaround = (double)totalT / completedCount;
    m->avgWaiting = (double)totalW / completedCount;

    if (lastFinish == 0)
        m->cpuUtilization = 0;
    else
        m->cpuUtilization = ((double)totalBurst / lastFinish) * 100.0;
}


