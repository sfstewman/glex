#ifndef GLEX_TESTS_H
#define GLEX_TESTS_H

#include <stdio.h>
#include <math.h>

#if GLEX_TEST_BYTESTREAM
struct bytestream {
  size_t loc;
  size_t len;
  unsigned char bytes[1024];
};

#define BYTESTREAM(s) { .loc = 0, .len = sizeof(s)-1, .bytes = s }

static int bytestream_getc(struct bytestream *ctx)
{
  if (ctx->loc >= ctx->len) { return -1; }

  return ctx->bytes[ctx->loc++];
}

static int bytestream_ungetc(int c, struct bytestream *ctx)
{
  if (ctx->loc > 0) { ctx->loc--; }
  return 0;
}
#endif /* GLEX_TEST_BYTESTREAM */

extern int glex_test_failures;
extern int glex_test_numtests;

#define EXPECT( val, expr ) do { \
  int expt_val = (val); int expt_act = (expr); \
  if (expt_val != expt_act) { \
    fprintf(stderr, "[%s:%d] EXPECT FAILED in %s\n" \
        "  expected %d from %s\n  actual %d from %s\n", \
        __FILE__, __LINE__, __func__, expt_val, #val, expt_act, #expr); \
    glex_test_failures++; return; } } while (0)

#define EXPECT_DBL( val, expr, tol ) do { \
  double expt_val = (val); double expt_act = (expr); \
  if (fabs(expt_val - expt_act) > (tol)) { \
    fprintf(stderr, "[%s:%d] EXPECT FAILED in %s\n" \
        "  expected %f from %s\n  actual %f from %s\n", \
        __FILE__, __LINE__, __func__, expt_val, #val, expt_act, #expr); \
    glex_test_failures++; return; } } while (0)

#define EXPECT_STR( val, expr ) do { \
  const char *expt_val = (const char*)(val); \
  const char *expt_act = (const char*)(expr); \
  if (strcmp(expt_val, expt_act) != 0) { \
    fprintf(stderr, "[%s:%d] EXPECT_STR FAILED in %s\n" \
        "  expected '%s' from %s\n  actual '%s' from %s\n", \
        __FILE__, __LINE__, __func__, expt_val, #val, expt_act, #expr); \
    glex_test_failures++; return; } } while (0)

#define DEFTEST( name ) \
  static void test_ ## name (void); \
  static void test_ ## name ## _run (void) { \
    glex_test_numtests++; test_ ## name (); \
  } \
  static void test_ ## name (void)

#define RUNTEST( name ) test_ ## name ## _run()



#endif /* GLEX_TESTS_H */

