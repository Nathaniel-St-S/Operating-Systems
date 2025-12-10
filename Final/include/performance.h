#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <stdint.h>
#include <time.h>
#include <sys/time.h>

// Maximum number of algorithms to track
#define MAX_ALGORITHMS 10
#define MAX_PROCESSES 100

// Performance metrics for individual processes
typedef struct {
  int pid;
  int arrival_time;
  int burst_time;
  int completion_time;
  int waiting_time;
  int turnaround_time;
  int response_time;
  int priority;
} ProcessMetrics;

// Performance metrics for scheduling algorithms
typedef struct {
  char algorithm_name[64];
  double execution_time;           // Time to complete all processes
  int context_switches;            // Number of context switches
  double avg_waiting_time;         // Average waiting time
  double avg_turnaround_time;      // Average turnaround time
  double avg_response_time;        // Average response time
  double cpu_utilization;          // CPU utilization percentage
  double throughput;               // Processes completed per unit time
  
  // Memory metrics
  unsigned long l1_cache_hits;
  unsigned long l1_cache_misses;
  unsigned long l2_cache_hits;
  unsigned long l2_cache_misses;
  unsigned long write_backs;
  
  // Per-process metrics
  ProcessMetrics process_metrics[MAX_PROCESSES];
  int process_count;
  
  // Timing breakdown
  double scheduler_time;           // Time spent in scheduler
  double context_switch_time;      // Time spent context switching
  double execution_time_total;     // Total process execution time
  double idle_time;                // CPU idle time
  
  // System time tracking
  int start_time;
  int end_time;
  int total_burst_time;
} PerformanceMetrics;

// Global metrics storage
typedef struct {
  PerformanceMetrics algorithms[MAX_ALGORITHMS];
  int algorithm_count;
  
  // System-wide cache stats (captured at start/end)
  unsigned long initial_l1_hits;
  unsigned long initial_l1_misses;
  unsigned long initial_l2_hits;
  unsigned long initial_l2_misses;
  unsigned long initial_write_backs;
} PerformanceTracker;

// Timer for measuring execution time
typedef struct {
  struct timespec start;
  struct timespec end;
  int is_running;
} PerfTimer;

// Initialize performance tracking system
void init_performance_tracking(void);

// Free performance tracking resources
void free_performance_tracking(void);

// Start tracking a new algorithm
int start_algorithm_tracking(const char *algorithm_name);

// End tracking for current algorithm
void end_algorithm_tracking(int algorithm_id);

// Record process metrics
void record_process_metrics(int algorithm_id, int pid, int arrival_time, 
                           int burst_time, int completion_time, 
                           int waiting_time, int turnaround_time,
                           int response_time, int priority);

// Record a context switch
void record_context_switch(int algorithm_id);

// Record scheduler time
void record_scheduler_time(int algorithm_id, double time_ms);

// Record context switch time
void record_context_switch_time(int algorithm_id, double time_ms);

// Capture memory statistics at end of algorithm
void capture_memory_stats(int algorithm_id);

// Calculate final metrics for an algorithm
void calculate_algorithm_metrics(int algorithm_id);

// Timer functions
void perf_timer_start(PerfTimer *timer);
double perf_timer_end(PerfTimer *timer); // Returns elapsed time in milliseconds
double perf_timer_end_seconds(PerfTimer *timer); // Returns elapsed time in seconds

// Print results
void print_algorithm_results(int algorithm_id);
void print_process_table(int algorithm_id);
void print_comparison_table(void);
void print_detailed_report(void);

// Export to CSV
void export_to_csv(const char *filename);
void export_comparison_csv(const char *filename);

// Visualization helpers (generates data for plotting)
void generate_chart_data(const char *chart_type, const char *output_file);

#endif // PERFORMANCE_H
