#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdint.h>

typedef void (*TestFunc)(void);

typedef struct TestCase {
  const char *suite;
  const char *name;
  TestFunc func;
} TestCase;

int register_test(const char *suite, const char *name, TestFunc func);
int run_all_tests(void);
void test_fail(const char *file, int line, const char *expr,
               int64_t actual, int64_t expected);

#define TEST_CASE(suite, name)                                                   \
  static void suite##_##name(void);                                             \
  static void suite##_##name##_register(void) __attribute__((constructor));     \
  static void suite##_##name##_register(void) {                                 \
    register_test(#suite, #name, suite##_##name);                               \
  }                                                                             \
  static void suite##_##name(void)

#define ASSERT_EQ(actual, expected)                                              \
  do {                                                                          \
    int64_t _actual = (int64_t)(actual);                                        \
    int64_t _expected = (int64_t)(expected);                                    \
    if (_actual != _expected) {                                                 \
      test_fail(__FILE__, __LINE__, #actual " == " #expected, _actual,         \
                _expected);                                                     \
      return;                                                                   \
    }                                                                           \
  } while (0)

#define ASSERT_TRUE(expr)                                                        \
  do {                                                                          \
    if (!(expr)) {                                                              \
      test_fail(__FILE__, __LINE__, #expr, 0, 1);                               \
      return;                                                                   \
    }                                                                           \
  } while (0)

#endif // TEST_FRAMEWORK_H
