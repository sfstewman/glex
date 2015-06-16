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

DEFTEST( bytestreamd_getc_and_ungetc )
{
  struct bytestream s = BYTESTREAM("foo");

  EXPECT( (int)'f', bytestream_getc(&s) );
  bytestream_ungetc('f', &s);

  EXPECT( (int)'f', bytestream_getc(&s) );
  bytestream_ungetc('f', &s);
}

DEFTEST( returns_eof_on_empty_stream )
{
  struct bytestream s = BYTESTREAM("");
  struct gen_lexer lexer;

  EXPECT( -1, bytestream_getc(&s) );

  gen_lexer_initialize(&lexer, &s);
  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_eof_on_whitespace_stream )
{
  struct bytestream s = BYTESTREAM("      ");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_id )
{
  struct bytestream s = BYTESTREAM("foo");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);
  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_several_ids )
{
  struct bytestream s = BYTESTREAM(" foo\n\n  bar\nbaz quux\n");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "bar", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "baz", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "quux", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_keywords_and_ids )
{
  struct bytestream s = BYTESTREAM(" foo  if  bar  while");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( KW_IF, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "if", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "bar", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( KW_WHILE, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "while", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_string )
{
  struct bytestream s = BYTESTREAM(
      " \"foo\\\\\\n  bar\\tbaz quux\\n\"\n\"");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_STRING_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "foo\\\n  bar\tbaz quux\n", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_ERR_UNEXPECTED_EOF, gen_lexer_next_token(&lexer) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_char )
{
  struct bytestream s = BYTESTREAM( " 'c'  '\n'  \"foo\" 'c '" );
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 'c', gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "c", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( '\n', gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "\n", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( GENLEX_STRING_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  /* " 'c'  '\n'  \"foo\" 'c '" */
  /*                        ^   */
  EXPECT( GENLEX_ERR_INVALID_CHAR, gen_lexer_next_token(&lexer) );

  /* " 'c'  '\n'  \"foo\" 'c '" */
  /*                          ^ */
  EXPECT( GENLEX_ERR_UNEXPECTED_EOF, gen_lexer_next_token(&lexer) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( returns_literals )
{
  struct bytestream s = BYTESTREAM( " 32 + 15 - 5 * (3 + 2);\nfoo(9);\nx = \"bar\";\n");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

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
}

DEFTEST( returns_int )
{
  struct bytestream s = BYTESTREAM( "1 32\t454\n4 20010343 100000000000000000000" );
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 1, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "1", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 32, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "32", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 454, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "454", gen_lexer_token_string(&lexer, NULL) );

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 4, gen_lexer_token_int_value(&lexer) );
  EXPECT_STR( "4", gen_lexer_token_string(&lexer, NULL) );

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

void run_all_tests_noopts(void)
{
  RUNTEST( bytestreamd_getc_and_ungetc );

  RUNTEST( returns_eof_on_empty_stream );
  RUNTEST( returns_eof_on_whitespace_stream );

  RUNTEST( returns_id );
  RUNTEST( returns_several_ids );
  RUNTEST( returns_keywords_and_ids );

  RUNTEST( returns_string );
  RUNTEST( returns_char   );
  RUNTEST( returns_int );

  RUNTEST( returns_literals );
}


