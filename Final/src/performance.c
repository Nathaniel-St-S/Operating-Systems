#include "../include/performance.h"
#include "../include/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

static PerformanceTracker *g_tracker = NULL;
static int g_current_algorithm = -1;

void init_performance_tracking(void) {
  g_tracker = calloc(1, sizeof(PerformanceTracker));
  if (!g_tracker) {
    fprintf(stderr, "Failed to allocate performance tracker\n");
    exit(EXIT_FAILURE);
  }
  g_tracker->algorithm_count = 0;
  g_current_algorithm = -1;
  
  printf("Performance tracking initialized\n");
}

void free_performance_tracking(void) {
  if (g_tracker) {
    free(g_tracker);
    g_tracker = NULL;
  }
}

int start_algorithm_tracking(const char *algorithm_name) {
  if (!g_tracker) {
    fprintf(stderr, "Performance tracker not initialized\n");
    return -1;
  }
  
  if (g_tracker->algorithm_count >= MAX_ALGORITHMS) {
    fprintf(stderr, "Maximum number of algorithms reached\n");
    return -1;
  }
  
  int id = g_tracker->algorithm_count++;
  g_current_algorithm = id;
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[id];
  memset(metrics, 0, sizeof(PerformanceMetrics));
  
  strncpy(metrics->algorithm_name, algorithm_name, 63);
  metrics->algorithm_name[63] = '\0';
  
  // Capture initial cache statistics
  // Note: You'll need to add functions to memory.c to get these values
  metrics->start_time = 0;
  
  printf("\n=== Starting performance tracking for: %s ===\n", algorithm_name);
  
  return id;
}

void end_algorithm_tracking(int algorithm_id) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[algorithm_id];
  
  // Capture final memory statistics
  capture_memory_stats(algorithm_id);
  
  // Calculate final metrics
  calculate_algorithm_metrics(algorithm_id);
  
  printf("=== Performance tracking completed for: %s ===\n\n", metrics->algorithm_name);
}

void record_process_metrics(int algorithm_id, int pid, int arrival_time,
                           int burst_time, int completion_time,
                           int waiting_time, int turnaround_time,
                           int response_time, int priority) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[algorithm_id];
  
  if (metrics->process_count >= MAX_PROCESSES) {
    fprintf(stderr, "Maximum number of processes reached for algorithm %d\n", algorithm_id);
    return;
  }
  
  ProcessMetrics *pm = &metrics->process_metrics[metrics->process_count++];
  pm->pid = pid;
  pm->arrival_time = arrival_time;
  pm->burst_time = burst_time;
  pm->completion_time = completion_time;
  pm->waiting_time = waiting_time;
  pm->turnaround_time = turnaround_time;
  pm->response_time = response_time;
  pm->priority = priority;
  
  metrics->total_burst_time += burst_time;
  
  // Update end time
  if (completion_time > metrics->end_time) {
    metrics->end_time = completion_time;
  }
}

void record_context_switch(int algorithm_id) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  g_tracker->algorithms[algorithm_id].context_switches++;
}

void record_scheduler_time(int algorithm_id, double time_ms) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  g_tracker->algorithms[algorithm_id].scheduler_time += time_ms;
}

void record_context_switch_time(int algorithm_id, double time_ms) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  g_tracker->algorithms[algorithm_id].context_switch_time += time_ms;
}

void capture_memory_stats(int algorithm_id) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[algorithm_id];
  
  metrics->l1_cache_hits   = get_L1_hits();
  metrics->l1_cache_misses = get_L1_misses();
  metrics->l2_cache_hits   = get_L2_hits();
  metrics->l2_cache_misses = get_L2_misses();
  metrics->write_backs     = get_write_backs();}

void calculate_algorithm_metrics(int algorithm_id) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[algorithm_id];
  
  if (metrics->process_count == 0) {
    return;
  }
  
  // Calculate average times
  double total_waiting = 0;
  double total_turnaround = 0;
  double total_response = 0;
  
  for (int i = 0; i < metrics->process_count; i++) {
    total_waiting += metrics->process_metrics[i].waiting_time;
    total_turnaround += metrics->process_metrics[i].turnaround_time;
    total_response += metrics->process_metrics[i].response_time;
  }
  
  metrics->avg_waiting_time = total_waiting / metrics->process_count;
  metrics->avg_turnaround_time = total_turnaround / metrics->process_count;
  metrics->avg_response_time = total_response / metrics->process_count;
  
  // Calculate CPU utilization
  int total_time = metrics->end_time - metrics->start_time;
  if (total_time > 0) {
    metrics->cpu_utilization = (double)metrics->total_burst_time / total_time * 100.0;
    metrics->idle_time = total_time - metrics->total_burst_time;
  } else {
    metrics->cpu_utilization = 100.0;
    metrics->idle_time = 0;
  }
  
  // Calculate throughput (processes per time unit)
  if (total_time > 0) {
    metrics->throughput = (double)metrics->process_count / total_time;
  }
  
  // Total execution time
  metrics->execution_time_total = total_time;
}

void perf_timer_start(PerfTimer *timer) {
  if (!timer) return;
  
  clock_gettime(CLOCK_MONOTONIC, &timer->start);
  timer->is_running = 1;
}

double perf_timer_end(PerfTimer *timer) {
  if (!timer || !timer->is_running) return 0.0;
  
  clock_gettime(CLOCK_MONOTONIC, &timer->end);
  timer->is_running = 0;
  
  double elapsed = (timer->end.tv_sec - timer->start.tv_sec) * 1000.0;
  elapsed += (timer->end.tv_nsec - timer->start.tv_nsec) / 1000000.0;
  
  return elapsed;
}

double perf_timer_end_seconds(PerfTimer *timer) {
  return perf_timer_end(timer) / 1000.0;
}

void print_process_table(int algorithm_id) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[algorithm_id];
  
  printf("\nPROCESS  ARRIVAL  BURST  COMPLETION  WAITING  TURNAROUND  RESPONSE  PRIORITY\n");
  printf("===============================================================================\n");
  
  for (int i = 0; i < metrics->process_count; i++) {
    ProcessMetrics *pm = &metrics->process_metrics[i];
    printf("P%-6d  %-7d  %-5d  %-10d  %-7d  %-10d  %-8d  %-8d\n",
           pm->pid,
           pm->arrival_time,
           pm->burst_time,
           pm->completion_time,
           pm->waiting_time,
           pm->turnaround_time,
           pm->response_time,
           pm->priority);
  }
  printf("===============================================================================\n");
}

void print_algorithm_results(int algorithm_id) {
  if (!g_tracker || algorithm_id < 0 || algorithm_id >= g_tracker->algorithm_count) {
    return;
  }
  
  PerformanceMetrics *metrics = &g_tracker->algorithms[algorithm_id];
  
  printf("\n");
  printf("========================================================================\n");
  printf("                    ALGORITHM: %s\n", metrics->algorithm_name);
  printf("========================================================================\n");
  
  printf("\nTiming Metrics:\n");
  printf("  Total Execution Time:      %.3f time units\n", metrics->execution_time_total);
  printf("  CPU Active Time:           %.3f time units\n", (double)metrics->total_burst_time);
  printf("  CPU Idle Time:             %.3f time units\n", metrics->idle_time);
  printf("  Scheduler Overhead:        %.3f ms\n", metrics->scheduler_time);
  printf("  Context Switch Overhead:   %.3f ms\n", metrics->context_switch_time);
  
  printf("\nProcess Metrics:\n");
  printf("  Number of Processes:       %d\n", metrics->process_count);
  printf("  Average Waiting Time:      %.3f\n", metrics->avg_waiting_time);
  printf("  Average Turnaround Time:   %.3f\n", metrics->avg_turnaround_time);
  printf("  Average Response Time:     %.3f\n", metrics->avg_response_time);
  
  printf("\nSystem Metrics:\n");
  printf("  CPU Utilization:           %.2f%%\n", metrics->cpu_utilization);
  printf("  Throughput:                %.3f processes/unit\n", metrics->throughput);
  printf("  Context Switches:          %d\n", metrics->context_switches);
  
  printf("\nMemory Statistics:\n");
  printf("  L1 Cache Hits:             %lu\n", metrics->l1_cache_hits);
  printf("  L1 Cache Misses:           %lu\n", metrics->l1_cache_misses);
  if (metrics->l1_cache_hits + metrics->l1_cache_misses > 0) {
    double l1_hit_rate = (double)metrics->l1_cache_hits / 
                         (metrics->l1_cache_hits + metrics->l1_cache_misses) * 100.0;
    printf("  L1 Hit Rate:               %.2f%%\n", l1_hit_rate);
  }
  printf("  L2 Cache Hits:             %lu\n", metrics->l2_cache_hits);
  printf("  L2 Cache Misses:           %lu\n", metrics->l2_cache_misses);
  if (metrics->l2_cache_hits + metrics->l2_cache_misses > 0) {
    double l2_hit_rate = (double)metrics->l2_cache_hits / 
                         (metrics->l2_cache_hits + metrics->l2_cache_misses) * 100.0;
    printf("  L2 Hit Rate:               %.2f%%\n", l2_hit_rate);
  }
  printf("  Write-Backs:               %lu\n", metrics->write_backs);
  
  print_process_table(algorithm_id);
  
  printf("\n");
}

void print_comparison_table(void) {
  if (!g_tracker || g_tracker->algorithm_count == 0) {
    printf("No algorithm data to compare\n");
    return;
  }
  
  printf("\n");
  printf("=====================================================================================================\n");
  printf("                              SCHEDULING ALGORITHM COMPARISON\n");
  printf("=====================================================================================================\n");
  printf("%-15s %10s %12s %12s %10s %12s\n",
         "Algorithm", "Avg Wait", "Avg T.Around", "Avg Resp", "CPU%%", "C.Switches");
  printf("-----------------------------------------------------------------------------------------------------\n");
  
  for (int i = 0; i < g_tracker->algorithm_count; i++) {
    PerformanceMetrics *m = &g_tracker->algorithms[i];
    printf("%-15s %10.3f %12.3f %12.3f %9.2f%% %12d\n",
           m->algorithm_name,
           m->avg_waiting_time,
           m->avg_turnaround_time,
           m->avg_response_time,
           m->cpu_utilization,
           m->context_switches);
  }
  printf("=====================================================================================================\n");
  
  // Find best performer in each category
  if (g_tracker->algorithm_count > 0) {
    int best_wait = 0, best_turnaround = 0, best_response = 0, best_cpu = 0, best_switches = 0;
    
    for (int i = 1; i < g_tracker->algorithm_count; i++) {
      if (g_tracker->algorithms[i].avg_waiting_time < 
          g_tracker->algorithms[best_wait].avg_waiting_time) {
        best_wait = i;
      }
      if (g_tracker->algorithms[i].avg_turnaround_time < 
          g_tracker->algorithms[best_turnaround].avg_turnaround_time) {
        best_turnaround = i;
      }
      if (g_tracker->algorithms[i].avg_response_time < 
          g_tracker->algorithms[best_response].avg_response_time) {
        best_response = i;
      }
      if (g_tracker->algorithms[i].cpu_utilization > 
          g_tracker->algorithms[best_cpu].cpu_utilization) {
        best_cpu = i;
      }
      if (g_tracker->algorithms[i].context_switches < 
          g_tracker->algorithms[best_switches].context_switches) {
        best_switches = i;
      }
    }
    
    printf("\nBest Performers:\n");
    printf("  Lowest Average Waiting Time:    %s (%.3f)\n",
           g_tracker->algorithms[best_wait].algorithm_name,
           g_tracker->algorithms[best_wait].avg_waiting_time);
    printf("  Lowest Average Turnaround Time: %s (%.3f)\n",
           g_tracker->algorithms[best_turnaround].algorithm_name,
           g_tracker->algorithms[best_turnaround].avg_turnaround_time);
    printf("  Lowest Average Response Time:   %s (%.3f)\n",
           g_tracker->algorithms[best_response].algorithm_name,
           g_tracker->algorithms[best_response].avg_response_time);
    printf("  Highest CPU Utilization:        %s (%.2f%%)\n",
           g_tracker->algorithms[best_cpu].algorithm_name,
           g_tracker->algorithms[best_cpu].cpu_utilization);
    printf("  Fewest Context Switches:        %s (%d)\n",
           g_tracker->algorithms[best_switches].algorithm_name,
           g_tracker->algorithms[best_switches].context_switches);
  }
  
  printf("\n");
}

void print_detailed_report(void) {
  if (!g_tracker) {
    printf("Performance tracker not initialized\n");
    return;
  }
  
  printf("\n");
  printf("################################################################################\n");
  printf("#                   DETAILED PERFORMANCE ANALYSIS REPORT                      #\n");
  printf("################################################################################\n");
  
  for (int i = 0; i < g_tracker->algorithm_count; i++) {
    print_algorithm_results(i);
  }
  
  print_comparison_table();
}

void export_to_csv(const char *filename) {
  if (!g_tracker) return;
  
  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for writing\n", filename);
    return;
  }
  
  fprintf(fp, "Algorithm,AvgWaitTime,AvgTurnaroundTime,AvgResponseTime,CPUUtilization,");
  fprintf(fp, "Throughput,ContextSwitches,L1Hits,L1Misses,L2Hits,L2Misses\n");
  
  for (int i = 0; i < g_tracker->algorithm_count; i++) {
    PerformanceMetrics *m = &g_tracker->algorithms[i];
    fprintf(fp, "%s,%.3f,%.3f,%.3f,%.2f,%.3f,%d,%lu,%lu,%lu,%lu\n",
            m->algorithm_name,
            m->avg_waiting_time,
            m->avg_turnaround_time,
            m->avg_response_time,
            m->cpu_utilization,
            m->throughput,
            m->context_switches,
            m->l1_cache_hits,
            m->l1_cache_misses,
            m->l2_cache_hits,
            m->l2_cache_misses);
  }
  
  fclose(fp);
  printf("Exported comparison data to %s\n", filename);
}

void export_comparison_csv(const char *filename) {
  export_to_csv(filename);
}

void generate_chart_data(const char *chart_type, const char *output_file) {
  if (!g_tracker) return;
  
  FILE *fp = fopen(output_file, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for writing\n", output_file);
    return;
  }
  
  if (strcmp(chart_type, "waiting_time") == 0) {
    fprintf(fp, "Algorithm,AvgWaitingTime\n");
    for (int i = 0; i < g_tracker->algorithm_count; i++) {
      fprintf(fp, "%s,%.3f\n",
              g_tracker->algorithms[i].algorithm_name,
              g_tracker->algorithms[i].avg_waiting_time);
    }
  } else if (strcmp(chart_type, "cpu_util") == 0) {
    fprintf(fp, "Algorithm,CPUUtilization\n");
    for (int i = 0; i < g_tracker->algorithm_count; i++) {
      fprintf(fp, "%s,%.2f\n",
              g_tracker->algorithms[i].algorithm_name,
              g_tracker->algorithms[i].cpu_utilization);
    }
  } else if (strcmp(chart_type, "context_switches") == 0) {
    fprintf(fp, "Algorithm,ContextSwitches\n");
    for (int i = 0; i < g_tracker->algorithm_count; i++) {
      fprintf(fp, "%s,%d\n",
              g_tracker->algorithms[i].algorithm_name,
              g_tracker->algorithms[i].context_switches);
    }
  }
  
  fclose(fp);
  printf("Generated chart data for %s in %s\n", chart_type, output_file);
}
