#include <stdio.h>

#undef GLEX_TEST_BYTESTREAM
#include "glex_tests.h"

#define GENLEX_GETC(ctx) (getc(ctx))
#define GENLEX_UNGETC(ch,ctx) (ungetc(ch,ctx))

/* Small so we can check that the bound is enforced */
#define GENLEX_STRING_MAX 64

#define GENLEX_IS_SYMBOL(ch,pos)  (isalpha(ch) || ((ch) == '_') || (((pos)>0) && isnumber(ch)))
#define GENLEX_LITERALS "()=;+-*/"

#define GENLEX_ID_TOKEN     1024
#define GENLEX_STRING_TOKEN 1025
#define GENLEX_INT_TOKEN    1026

#define KW_IF    1027
#define KW_WHILE 1028

#define GENLEX_KEYWORDS { \
  { "if"   , KW_IF    }, \
  { "while", KW_WHILE }, \
}

#include "glex.h"

/* glex_test_stdio.c : runs some limited tests with stdio getc/ungetc
 * instead of the bytestream one used in most testing
 */

void test_stdio_inner(FILE *f)
{
  struct gen_lexer lexer;

  fputs( " 32 + 15 - 5 * (3 + 2);\nfoo(9);\nx = \"bar\";\n", f);
  rewind(f);

  gen_lexer_initialize(&lexer, f);

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 32, gen_lexer_token_int_value(&lexer) );

  EXPECT( '+', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 15, gen_lexer_token_int_value(&lexer) );

  EXPECT( '-', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 5, gen_lexer_token_int_value(&lexer) );

  EXPECT( '*', gen_lexer_next_token(&lexer) );
  EXPECT( '(', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 3, gen_lexer_token_int_value(&lexer) );

  EXPECT( '+', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 2, gen_lexer_token_int_value(&lexer) );

  EXPECT( ')', gen_lexer_next_token(&lexer) );
  EXPECT( ';', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( '(', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 9, gen_lexer_token_int_value(&lexer) );

  EXPECT( ')', gen_lexer_next_token(&lexer) );
  EXPECT( ';', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "x", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( '=', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_STRING_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "bar", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( ';', gen_lexer_next_token(&lexer) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );

  fclose(f);
}

DEFTEST( test_stdio )
{
  FILE *f;
  f = tmpfile();
  test_stdio_inner(f);
  fclose(f);
}

void run_tests_stdio(void)
{
  /* FIXME: test these! */
  (void)gen_lexer_token_line;
  (void)gen_lexer_token_col;
  (void)gen_lexer_token_off;
  RUNTEST( test_stdio );
}

