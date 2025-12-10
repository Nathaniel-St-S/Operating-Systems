#ifndef OS_H
#define OS_H

#include <stdio.h>
#include <time.h>
#include <pthread.h>

//constants

#define RAM_SIZE 1024
#define L1_SIZE 64
#define L2_SIZE 128
#define BLOCK_SIZE 100
#define NUM_BLOCKS (RAM_SIZE / BLOCK_SIZE)
#define STACK_SIZE 10

// for scheduler
#define MAX_PROCESSES 10

// enums

// opcodes
typedef enum {
    HALT = 0, ADD, SUB, LOAD, STORE, MULT, DIV, AND, OR, JMP, JZ
} OpCode;

// 
typedef enum {
    READY = 0,
    RUNNING = 1,
    BLOCKED = 2
} ProcessState;

typedef enum { THROUGH = 0, BACK = 1 } Policy;
typedef enum { FF = 0, BF = 1 } AllocStrat;

// for cpu flags
typedef struct {
    int zeroFlag;
    int carryFlag;
    int overflowFlag;
} StatusRegister;

// cpu structure
typedef struct {
    int PC;
    int ACC;
    OpCode IR;
    int status; // halted = 1
    StatusRegister statusRegister;
} CPU;

// memory structures

typedef struct {
    int valid;
    int dirty;
    int address;
    int value;
} CacheLine;

typedef struct {
    int RAM[RAM_SIZE];
    CacheLine L1[L1_SIZE];
    CacheLine L2[L2_SIZE];
    int hitsL1, missesL1;
    int hitsL2, missesL2;
    int nextL1, nextL2;
    int cachePolicy;
} MemorySystem;


typedef struct {
    int processID;
    int memoryStart;
    int memoryEnd;
    int isFree;
} MemoryBlock;


typedef struct PCB {

    // saved cpu state
    int pid;
    int PC;
    int ACC;
    OpCode IR;
    StatusRegister flags;

    // for scheduling fields 
    ProcessState state;
    int arrivalTime;
    int burstTime;
    int originalBurst;
    int waitTime;
    int serviceTime;
    float responseRatio;
    int priority;

    // for metrics
    int startTime;
    int finishTime;

} PCB;

// global simulator state

// cpu module state
extern PCB processTable[MAX_PROCESSES];
extern int currentProcess;
extern int interruptFlag;

extern MemoryBlock memoryTable[NUM_BLOCKS];

// scheduler module state
extern PCB runningProcess;

extern PCB pcbTable[MAX_PROCESSES + 1];

extern PCB readyQueue[MAX_PROCESSES];
extern int readyCount, readyHead, readyTail;

extern PCB blockedQueue[MAX_PROCESSES];
extern int blockedCount, blockedHead, blockedTail;

extern PCB completedTable[MAX_PROCESSES + 1];
extern int completedCount;

extern int currTime;
extern int timeQuantumGlobal;
extern int queueCapacity; // runtime-configurable effective queue size (<= MAX_PROCESSES)

// interrupt table
typedef void (*HandlerFunctionPointer)(void);
extern HandlerFunctionPointer interruptHandlerArray[4];

// cpu
int fetch(CPU *cpu, MemorySystem *mem);
void execute(CPU *cpu, MemorySystem *mem);
void updateFlags(CPU *cpu, int accBefore, int operand, int result, char op);
void updateZeroFlag(CPU *cpu);
void print_state(const CPU *cpu);

// memory
void initMemory(MemorySystem *mem);
int readMemory(MemorySystem *mem, int address);
void writeMemory(MemorySystem *mem, int address, int value);
void writeThrough(MemorySystem *mem, int address, int value);
void writeBack(MemorySystem *mem, int address, int value);

// dma
void dmaTransfer(MemorySystem *mem, int source, int dest, int size);
void initiateDMA(MemorySystem *mem, int source, int dest, int size);

// memory management
void initMemoryTable(void);
void allocateMemory(int pid, int size, int strategy);
void firstFitAllocate(int pid, int blocks);
void bestFitAllocate(int pid, int blocks);
void deallocateMemory(int pid);
void printMemoryTable(void);

// pcb
void updateProcessState(PCB *pcb, CPU *cpu);
void saveContext(CPU *cpu, PCB *pcb);
void loadContext(CPU *cpu, PCB *pcb);
void contextSwitch(CPU *cpu);
void initProcesses(void);
void runScheduler(CPU *cpu, MemorySystem *mem);

// interrupts
void timerInterrupt(void);
void ioInterrupt(void);
void systemCallInterrupt(void);
void trapInterrupt(void);
void priorityInterrupt(void);
void interruptHandler(CPU *cpu, void *stack);
void checkForInterrupt(CPU *cpu, void *stack);
void initIVT(void);

// dispatcher - select next ready process to run
void dispatcher(void);

// scheduler queues
void enqueueReadyProcess(PCB p);
PCB dequeueReadyProcess(void);

void enqueueBlockedProcess(PCB p);
PCB dequeueBlockedProcess(void);

void saveState(PCB *p);
void restoreState(PCB *p);

void runInstruction(PCB *p);
void recordFinished(PCB p);

// scheduling alg
void roundRobinScheduler(int quantum);
void priorityScheduler(void);
void shortestProcessNextScheduler(void);
void srtfScheduler(void);
void hrrnScheduler(void);
void feedbackScheduler(void);
// threads
void startInterruptSimulatorThread(void);
void startSchedulerMonitorThread(void);
void startMemoryMonitorThread(void);
void joinOptionalThreads(void);
void fcfsScheduler(void);

// performance

extern int perf_contextSwitches;

typedef struct {
    double execTime;
    int contextSwitches;
    double avgWaiting;
    double avgTurnaround;
    double cpuUtilization;
} Metrics;

void perf_resetCounters(void);
void perf_beginTimer(void);
void perf_endTimer(Metrics *m);
void perf_finalizeMetrics(Metrics *m);

#endif 

