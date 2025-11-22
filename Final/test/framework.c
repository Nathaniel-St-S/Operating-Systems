#include "framework.h"

#include <stdio.h>
#include <stdlib.h>

#define MAX_TESTS 256

static TestCase g_tests[MAX_TESTS];
static size_t g_test_count;
static const char *g_current_suite;
static const char *g_current_name;
static int g_current_failed;

int register_test(const char *suite, const char *name, TestFunc func) {
  if (g_test_count >= MAX_TESTS) {
    fprintf(stderr, "Maximum number of tests exceeded\n");
    exit(EXIT_FAILURE);
  }
  g_tests[g_test_count++] = (TestCase){suite, name, func};
  return 0;
}

void test_fail(const char *file, int line, const char *expr,
               int64_t actual, int64_t expected) {
  g_current_failed = 1;
  fprintf(stderr,
          "[FAIL] %s.%s: %s\n        at %s:%d\n        got: %lld (0x%llx)\n        exp: %lld (0x%llx)\n",
          g_current_suite, g_current_name, expr, file, line,
          (long long)actual, (unsigned long long)((uint64_t)actual & 0xFFFFFFFFFFFFFFFFULL),
          (long long)expected,
          (unsigned long long)((uint64_t)expected & 0xFFFFFFFFFFFFFFFFULL));
}

int run_all_tests(void) {
  size_t passed = 0;
  size_t failed = 0;

  for (size_t i = 0; i < g_test_count; ++i) {
    g_current_suite = g_tests[i].suite;
    g_current_name = g_tests[i].name;
    g_current_failed = 0;
    g_tests[i].func();
    if (g_current_failed) {
      ++failed;
    } else {
      ++passed;
    }
  }

  printf("Test results: %zu passed, %zu failed\n", passed, failed);
  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
