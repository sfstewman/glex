#include <stdlib.h>
#include <stdio.h>

#include "glex_tests.h"

int glex_test_failures = 0;

extern void run_all_tests_noopts(void);

int main(int argc, const char **argv)
{
  /* avoid warnings */
  (void)bytestream_getc;
  (void)bytestream_ungetc;

  run_all_tests_noopts();

  if (glex_test_failures > 0) {
    printf("There were %d failures in the tests.\n", glex_test_failures);
    return EXIT_FAILURE;
  }

  printf("all tests successful\n");
  return EXIT_SUCCESS;
}
