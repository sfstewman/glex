#include <stdlib.h>
#include <stdio.h>

#undef GLEX_TEST_BYTESTREAM
#include "glex_tests.h"

int glex_test_failures = 0;
int glex_test_numtests = 0;

extern void run_all_tests_noopts(void);
extern void run_tests_stdio(void);
extern void run_tests_numbers(void);

int main(int argc, const char **argv)
{
  run_all_tests_noopts();
  run_tests_stdio();
  run_tests_numbers();

  printf("%d tests run... ", glex_test_numtests);
  if (glex_test_failures > 0) {
    printf("%d failures in the tests.\n", glex_test_failures);
    return EXIT_FAILURE;
  }

  printf("all tests successful\n");
  return EXIT_SUCCESS;
}
