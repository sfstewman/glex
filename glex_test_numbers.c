#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define GLEX_TEST_BYTESTREAM 1
#include "glex_tests.h"

#define GENLEX_GETC(ctx) (bytestream_getc(ctx))
#define GENLEX_UNGETC(ch,ctx) (bytestream_ungetc(ch,ctx))

/* Small so we can check that the bound is enforced */
#define GENLEX_STRING_MAX 64

#define GENLEX_IS_SYMBOL(ch,pos)  (isalpha(ch) || ((ch) == '_') || (((pos)>0) && isnumber(ch)))
#define GENLEX_LITERALS "()"

#define GENLEX_ID_TOKEN     1024
#define GENLEX_STRING_TOKEN 1025
#define GENLEX_INT_TOKEN    1026
#define GENLEX_FLOAT_TOKEN  1027

#define GENLEX_KEYWORDS {}

#define GENLEX_CONFIG_FLOATS      1  /* enable float parsing */

#include "glex.h"

DEFTEST( returns_incomplete_int )
{
  struct bytestream s = BYTESTREAM( "-" );
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_ERR_INVALID_INTEGER, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_int )
{
  struct bytestream s = BYTESTREAM( "1 -32\t454\n -4 20010343 100000000000000000000" );
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 1, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "1", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( -32, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "-32", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 454, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "454", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( -4, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "-4", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 20010343, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "20010343", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_ERR_INTEGER_OVERFLOW, gen_lexer_next_token(&lexer) );
  /* on integer overflow or invalid integer errors, the string
   * representation should be stored in the lexer's token string
   */
  EXPECT_STR( "100000000000000000000", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_floats_and_ints )
{
  struct bytestream s = BYTESTREAM( "1.0 3. 1e-2 1.2e-2 23 (0.25) (1) 0.25q 15t" );
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_FLOAT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_DBL( 1.0, gen_lexer_token_float_value(&lexer), 0.0 );
  EXPECT_STR( "1.0", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_FLOAT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_DBL( 3.0, gen_lexer_token_float_value(&lexer), 0.0 );
  EXPECT_STR( "3.", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_FLOAT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_DBL( 1e-2, gen_lexer_token_float_value(&lexer), 1e-8 );
  EXPECT_STR( "1e-2", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_FLOAT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_DBL( 1.2e-2, gen_lexer_token_float_value(&lexer), 1e-8 );
  EXPECT_STR( "1.2e-2", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 23, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "23", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( '(', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_FLOAT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_DBL( 0.25, gen_lexer_token_float_value(&lexer), 1e-8 );
  EXPECT_STR( "0.25", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( ')', gen_lexer_next_token(&lexer) );

  EXPECT( '(', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 1, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "1", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( ')', gen_lexer_next_token(&lexer) );

  /* errors: symbol stuff after the integers */
  EXPECT( GENLEX_ERR_INVALID_CHAR, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "0.25", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_ERR_INVALID_CHAR, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "15", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

void run_tests_numbers(void)
{
  /* FIXME: test these! */
  (void)gen_lexer_token_line;
  (void)gen_lexer_token_col;
  (void)gen_lexer_token_off;

  RUNTEST( returns_incomplete_int );
  RUNTEST( returns_int );
  RUNTEST( returns_floats_and_ints );
}


