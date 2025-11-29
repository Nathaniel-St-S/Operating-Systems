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

typedef enum {
  CACHE_WRITE_THROUGH,
  CACHE_WRITE_BACK
} CachePolicy;

typedef struct {
  CachePolicy cache_policy;
  Scheduler scheduler;
  const char **program_files;
  int program_count;
} Options;

static Options opts = {
  .cache_policy = CACHE_WRITE_THROUGH,
  .scheduler = NULL,
  .program_files = NULL,
  .program_count = 0
};

static AssemblyResult *results;

static void parse_args(int argc, char *argv[]);

static void print_usage(const char *prog_name);

static void ensure_scheduler_selected(void);

static void run_program(const AssemblyResult *result, int pid);

// For error handling since we lowkey requesting a lot of memory now
static jmp_buf g_panic_buffer;
static volatile sig_atomic_t g_panic_handler_active = 0;

// For error check
static int result_count = 0;
static bool memory_initialized = false;

static void panic_handler(int sig);

int main(int argc, char *argv[])
{
  argc = 2;
  argv[0] = "programs/factorial.asm";
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

  ensure_scheduler_selected();
  if (opts.scheduler.init) {
    opts.scheduler.init();
  }

  int programs_ran = 0;
  for (int i = 0; i < opts.program_count; i++) {
    if (!results[i].success) {
      continue;
    }
    mallocate(i, 1000);
    run_program(&results[i], i);
    programs_ran++;
  }

  if (programs_ran == 0) {
    fprintf(stderr, "No runnable programs were found after assembly\n");
    exit_code = EXIT_FAILURE;
    goto cleanup;
  }

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
      opts.scheduler = fcfs;
    }
    else if (strcmp(argv[i], "--round-robin") == 0) {
      opts.scheduler = round_robin;
    }
    else if (strcmp(argv[i], "--priority") == 0) {
      opts.scheduler = priority;
    }
    else if (strcmp(argv[i], "--srt") == 0) {
      opts.scheduler = shortest_time_remaining;
    }
    else if (strcmp(argv[i], "--hrrn") == 0) {
      opts.scheduler = highest_response_ratio_next;
    }
    else if (strcmp(argv[i], "--spn") == 0) {
      opts.scheduler = shortest_process_next;
    }
    else if (strcmp(argv[i], "--mlfq") == 0) {
      opts.scheduler = multilevel_feedback_queue;
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

static void ensure_scheduler_selected(void) {
  if (opts.scheduler.init) {
    return;
  }

  fprintf(stdout,
          "No scheduling algorithm selected - defaulting to Round Robin\n");
  opts.scheduler = round_robin;
}

static void run_program(const AssemblyResult *result, int pid) {
  if (!result || !result->success || !result->program) {
    return;
  }

  printf("\n=== Starting program %s (PID %d) ===\n",
         result->program->program_name, pid);

  set_current_process(pid);

  init_cpu(result->program->entry_point);
  GP_REGISTER(REG_GP) = result->program->globl_ptr;
  GP_REGISTER(REG_SP) = result->program->stack_ptr;

  cpu_run();

  printf("=== Program %s completed ===\n", result->program->program_name);

  set_current_process(SYSTEM_PROCESS_ID);
}

static void print_usage(const char *prog_name) {
  // program name will be argv[0]
  printf("Usage: %s [OPTIONS] <program1.asm> <program2.asm> ...\n\n"
          "OPTIONS:\n"
          "  --write-through       Use write-through cache policy (default)\n"
          "  --write-back          Use write-back cache policy\n"
          "\n"
          "  --fcfs                First-Come First-Served scheduling\n"
          "  --round-robin         Round Robin scheduling (default)\n"
          "  --priority            Priority scheduling\n"
          "  --srt                 Shortest Remaining Time scheduling\n"
          "  --hrrn                Highest Response Ratio Next scheduling\n"
          "  --spn                 Shortest Process Next scheduling\n"
          "  --feedback            Multi-Level Feedback Queue scheduling\n"
          "\n"
          "EXAMPLES:\n"
          "  %s programs/hello_world.asm\n"
          "  %s --write-back --fcfs prog1.asm prog2.asm\n"
          "  %s --round-robin programs/*.asm"
          , prog_name, prog_name,  prog_name, prog_name);
}

void panic_handler(int sig) {
  if (g_panic_handler_active) {
    fprintf(stderr, "\nFATAL: Signal %d during panic cleanup - aborting\n", sig);
    abort();
  }

  fprintf(stderr, "\nPANIC: Caught signal %d\n", sig);
  longjmp(g_panic_buffer, sig);
}
