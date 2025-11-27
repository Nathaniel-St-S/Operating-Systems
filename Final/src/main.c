#include "../include/cpu.h"
#include "../include/memory.h"
#include "../include/assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>

#define MAX_PROGRAMS 10

typedef enum {
  CACHE_WRITE_THROUGH,
  CACHE_WRITE_BACK
} CachePolicy;

typedef enum {
  SCHED_FCFS,
  SCHED_ROUND_ROBIN,
  SCHED_PRIORITY,
  SCHED_SRT,
  SCHED_HRRN,
  SCHED_SPN,
  SCHED_MLFQ
} SchedulingAlgorithm;

typedef struct {
  CachePolicy cache_policy;
  SchedulingAlgorithm scheduler;
  const char **program_files;
  int program_count;
} Options;

static Options opts = {
  .cache_policy = CACHE_WRITE_THROUGH,
  .scheduler = SCHED_ROUND_ROBIN,
  .program_files = NULL,
  .program_count = 0
};

static AssemblyResult *results;

static void parse_args(int argc, char *argv[]);

static void print_usage(const char *prog_name);

// For error handling since we lowkey requesting a lot of memory now
static jmp_buf g_panic_buffer;
static volatile sig_atomic_t g_panic_handler_active = 0;

// For error check
static int result_count = 0;
static bool memory_initialized = false;

static void panic_handler(int sig);

int main(int argc, char *argv[])
{
  int exit_code = EXIT_SUCCESS;

  // Panic handlers for signalls
  signal(SIGSEGV, panic_handler); // Seg Fault
  signal(SIGABRT, panic_handler); // Abort (we don't use it yet but i will be in the future)
  signal(SIGFPE, panic_handler);  // Floating Point Error (div by 0)

  int panic_signal = setjmp(g_panic_buffer);
  if (panic_signal != 0) {
    fprintf(stderr, "Performing emergency cleanup after signal %d\n", panic_signal);
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  parse_args(argc, argv);

  init_memory();
  memory_initialized = true;

  results = calloc(opts.program_count, sizeof(AssemblyResult));
  result_count = opts.program_count;
  if(!results){
    fprintf(stderr, "Memory allocation failed for assembly results during initialization\n");
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  int success_count = assemble_programs(opts.program_files, 
                                        opts.program_count, 
                                        results);

  if (success_count == 0) {
    fprintf(stderr, "\nError: No programs assembled successfully\n");
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

  // Write main code here


  // Part of the cleanup process
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

  if (memory_initialized) {
    free_memory();
    memory_initialized = false;
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
  if (!opts.program_files) {
    fprintf(stderr, "Memory allocation failed for program files during cla parsing\n");
    exit(EXIT_FAILURE);
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
      opts.program_files[opts.program_count++] = argv[i];
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
  // program name will be argv[0]
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
  printf("  --feedback            Multi-Level Feedback Queue scheduling\n");
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
