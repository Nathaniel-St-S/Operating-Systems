#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/assembler.h"
#include "../include/processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#define MAX_PROGRAMS 10

typedef struct {
  CachePolicy cache_policy;
  SchedulingAlgorithm scheduler;
  const char **program_files;
  int program_count;
  int *priorities;        // Priority for each program
  int *burst_estimates;   // Burst time estimates for each program
} Options;

static Options opts = {
  .cache_policy = CACHE_WRITE_THROUGH,
  .scheduler = SCHED_ROUND_ROBIN,
  .program_files = NULL,
  .program_count = 0,
  .priorities = NULL,
  .burst_estimates = NULL
};

static AssemblyResult *results;

static void parse_args(int argc, char *argv[]);
static void print_usage(const char *prog_name);

static jmp_buf g_panic_buffer;
static volatile sig_atomic_t g_panic_handler_active = 0;

static int result_count = 0;
static bool memory_initialized = false;
static bool queues_initialized = false;

static void panic_handler(int sig);

int main(int argc, char *argv[])
{
  int exit_code = EXIT_SUCCESS;

  signal(SIGSEGV, panic_handler);
  signal(SIGABRT, panic_handler);
  signal(SIGFPE, panic_handler);

  int panic_signal = setjmp(g_panic_buffer);
  if (panic_signal != 0) {
    fprintf(stderr, "Performing emergency cleanup after signal %d\n", panic_signal);
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  parse_args(argc, argv);

  // Initialize memory system
  printf("Initializing memory system...\n");
  init_memory(opts.cache_policy);
  memory_initialized = true;

  // Initialize process queues
  printf("Initializing process queues...\n");
  init_queues();
  queues_initialized = true;

  // Allocate results array
  results = calloc(opts.program_count, sizeof(AssemblyResult));
  result_count = opts.program_count;
  if(!results){
    fprintf(stderr, "Memory allocation failed for assembly results\n");
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  // Assemble all programs and create processes
  printf("\n=== Assembling Programs ===\n");
  for (int i = 0; i < opts.program_count; i++) {
    printf("\n[%d/%d] Processing: %s\n", i+1, opts.program_count, opts.program_files[i]);
    
    // Assemble the program (this allocates memory and writes code/data)
    results[i] = assemble(opts.program_files[i], i);
    
    if (!results[i].success) {
      fprintf(stderr, "  ✗ Assembly failed: %s\n", results[i].error_message);
      continue;
    }
    
    printf("  ✓ Assembly successful\n");
    
    // Create process from assembled program
    uint32_t process_addr = makeProcess(
      i,                                      // Process ID
      results[i].program->entry_point,        // Entry point (PC)
      results[i].program->text_start,         // Text segment start
      results[i].program->text_size,          // Text segment size
      results[i].program->data_start,         // Data segment start
      results[i].program->data_size,          // Data segment size
      results[i].program->stack_ptr,          // Stack pointer
      opts.priorities[i],                     // Priority
      opts.burst_estimates[i]                 // Burst time estimate
    );
    
    if (process_addr == UINT32_MAX) {
      fprintf(stderr, "  ✗ Failed to create process\n");
      exit_code = EXIT_FAILURE;
    } else {
      printf("  ✓ Process created (PID: %d, Entry: 0x%08x)\n", 
             i, results[i].program->entry_point);
    }
  }

  // Run the scheduler
  printf("\n=== Starting Scheduler ===\n");
  printf("Algorithm: ");
  switch (opts.scheduler) {
    case SCHED_FCFS: printf("First-Come First-Served\n"); break;
    case SCHED_ROUND_ROBIN: printf("Round Robin\n"); break;
    case SCHED_PRIORITY: printf("Priority\n"); break;
    case SCHED_SRT: printf("Shortest Remaining Time\n"); break;
    case SCHED_HRRN: printf("Highest Response Ratio Next\n"); break;
    case SCHED_SPN: printf("Shortest Process Next\n"); break;
    case SCHED_MLFQ: printf("Multi-Level Feedback Queue\n"); break;
  }
  printf("\n");

  scheduler(opts.scheduler);

  printf("\n=== Execution Complete ===\n");
  print_cache_stats();

cleanup:
  g_panic_handler_active = 1;

  if (results) {
    for (int i = 0; i < result_count; i++) {
      if (results[i].success) {
        free_program(&results[i]);
      }
    }
    free(results);
    results = NULL;
  }

  if (opts.program_files) {
    free((void*)opts.program_files);
    opts.program_files = NULL;
  }

  if (opts.priorities) {
    free(opts.priorities);
    opts.priorities = NULL;
  }

  if (opts.burst_estimates) {
    free(opts.burst_estimates);
    opts.burst_estimates = NULL;
  }

  if (memory_initialized) {
    free_memory();
    memory_initialized = false;
  }

  if (queues_initialized){
    free_queues();
    queues_initialized = false;
  }

  g_panic_handler_active = 0;
  return exit_code;
}

static void parse_args(int argc, char* argv[]){
  if (argc < 2) {
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  opts.program_files = malloc(sizeof(char*) * argc);
  opts.priorities = malloc(sizeof(int) * argc);
  opts.burst_estimates = malloc(sizeof(int) * argc);
  
  if (!opts.program_files || !opts.priorities || !opts.burst_estimates) {
    fprintf(stderr, "Memory allocation failed during argument parsing\n");
    exit(EXIT_FAILURE);
  }

  // Default values for each program
  for (int i = 0; i < argc; i++) {
    opts.priorities[i] = 5;        // Default medium priority
    opts.burst_estimates[i] = 100; // Default burst estimate
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--write-through") == 0) {
      opts.cache_policy = CACHE_WRITE_THROUGH;
    }
    else if (strcmp(argv[i], "--write-back") == 0) {
      opts.cache_policy = CACHE_WRITE_BACK;
    }
    else if (strcmp(argv[i], "--fcfs") == 0) {
      opts.scheduler = SCHED_FCFS;
    }
    else if (strcmp(argv[i], "--round-robin") == 0) {
      opts.scheduler = SCHED_ROUND_ROBIN;
    }
    else if (strcmp(argv[i], "--priority") == 0) {
      opts.scheduler = SCHED_PRIORITY;
    }
    else if (strcmp(argv[i], "--srt") == 0) {
      opts.scheduler = SCHED_SRT;
    }
    else if (strcmp(argv[i], "--hrrn") == 0) {
      opts.scheduler = SCHED_HRRN;
    }
    else if (strcmp(argv[i], "--spn") == 0) {
      opts.scheduler = SCHED_SPN;
    }
    else if (strcmp(argv[i], "--mlfq") == 0) {
      opts.scheduler = SCHED_MLFQ;
    }
    else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      print_usage(argv[0]);
      exit(EXIT_SUCCESS);
    }
    else if (argv[i][0] == '-') {
      fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    }
    else {
      opts.program_files[opts.program_count] = argv[i];
      
      // Assign priority based on order (first = highest priority)
      opts.priorities[opts.program_count] = opts.program_count + 1;
      
      // Estimate burst time (can be refined later)
      opts.burst_estimates[opts.program_count] = 50 + (opts.program_count * 25);
      
      opts.program_count++;
    }
  }

  if (opts.program_count == 0) {
    fprintf(stderr, "Error: No program files specified\n\n");
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (opts.program_count > MAX_PROGRAMS) {
    fprintf(stderr, "Warning: Too many programs (%d), limiting to %d\n", 
        opts.program_count, MAX_PROGRAMS);
    opts.program_count = MAX_PROGRAMS;
  }
}

static void print_usage(const char *prog_name) {
  printf("Usage: %s [OPTIONS] <program1.asm> <program2.asm> ...\n\n", prog_name);
  printf("OPTIONS:\n");
  printf("  --write-through       Use write-through cache policy (default)\n");
  printf("  --write-back          Use write-back cache policy\n");
  printf("\n");
  printf("  --fcfs                First-Come First-Served scheduling\n");
  printf("  --round-robin         Round Robin scheduling (default)\n");
  printf("  --priority            Priority scheduling\n");
  printf("  --srt                 Shortest Remaining Time scheduling\n");
  printf("  --hrrn                Highest Response Ratio Next scheduling\n");
  printf("  --spn                 Shortest Process Next scheduling\n");
  printf("  --mlfq                Multi-Level Feedback Queue scheduling\n");
  printf("\n");
  printf("EXAMPLES:\n");
  printf("  %s programs/hello_world.asm\n", prog_name);
  printf("  %s --write-back --fcfs prog1.asm prog2.asm\n", prog_name);
  printf("  %s --round-robin programs/*.asm\n", prog_name);
}

void panic_handler(int sig) {
  if (g_panic_handler_active) {
    fprintf(stderr, "\nFATAL: Signal %d during panic cleanup - aborting\n", sig);
    abort();
  }

  fprintf(stderr, "\nPANIC: Caught signal %d\n", sig);
  longjmp(g_panic_buffer, sig);
}
