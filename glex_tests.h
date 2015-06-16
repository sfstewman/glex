#ifndef GLEX_TESTS_H
#define GLEX_TESTS_H

#include <stdio.h>

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

extern int glex_test_failures;

#define EXPECT( val, expr ) do { \
  int expt_val = (val); int expt_act = (expr); \
  if (expt_val != expt_act) { \
    fprintf(stderr, "[%s:%d] EXPECT FAILED in %s\n" \
        "  expected %d from %s\n  actual %d from %s\n", \
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

#define DEFTEST( name ) static void test_ ## name (void)
#define RUNTEST( name ) test_ ## name ()



#endif /* GLEX_TESTS_H */

