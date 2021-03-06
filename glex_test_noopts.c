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

#define GENLEX_ID_TOKEN      1024
#define GENLEX_STRING_TOKEN  1025
#define GENLEX_INT_TOKEN     1026
#define GENLEX_COMMENT_TOKEN 1027

#define KW_IF    1028
#define KW_WHILE 1029

#define LIT_EQ 512

#define GENLEX_LITERAL_PAIRS { \
  { "==", LIT_EQ },            \
}

#define GENLEX_KEYWORDS { \
  { "if"   , KW_IF    }, \
  { "while", KW_WHILE }, \
}

/* We do configure one option: comments. */
#define GENLEX_COMMENT_PAIRS GENLEX_C99_COMMENTS
 
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

DEFTEST( returns_literal_pairs )
{
  struct bytestream s = BYTESTREAM( " a = ( 32 == 15 - (-17) );" );
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "a", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( '=', gen_lexer_next_token(&lexer) );

  EXPECT( '(', gen_lexer_next_token(&lexer) );
  {
    EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
    EXPECT( 32, gen_lexer_token_int_value(&lexer) );

    EXPECT( LIT_EQ, gen_lexer_next_token(&lexer) );
    EXPECT_STR( "==", gen_lexer_token_string(&lexer,NULL) );

    EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
    EXPECT( 15, gen_lexer_token_int_value(&lexer) );

    EXPECT( '-', gen_lexer_next_token(&lexer) );

    EXPECT( '(', gen_lexer_next_token(&lexer) );
    {
      EXPECT( '-', gen_lexer_next_token(&lexer) );
      EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
      EXPECT( 17, gen_lexer_token_int_value(&lexer) );
    }
    EXPECT( ')', gen_lexer_next_token(&lexer) );
  }
  EXPECT( ')', gen_lexer_next_token(&lexer) );
  EXPECT( ';', gen_lexer_next_token(&lexer) );

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

DEFTEST( comments )
{
  struct gen_lexer lexer;
  struct bytestream s = BYTESTREAM(
      " 32 + /* yes */ 15 - 5 * (3 + 2); // this\n"
      "foo(9);\n"
      "x = \"bar /* not a comment */\";\n"
      );

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_INT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT( 32, gen_lexer_token_int_value(&lexer) );

  EXPECT( '+', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_COMMENT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( " yes ", gen_lexer_token_string(&lexer, NULL) );

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

  EXPECT( GENLEX_COMMENT_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( " this", gen_lexer_token_string(&lexer, NULL) );

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
  EXPECT_STR( "bar /* not a comment */", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( ';', gen_lexer_next_token(&lexer) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

DEFTEST( lexer_buffer_overflow_has_graceful_recovery )
{
  struct gen_lexer lexer;
  struct bytestream s = BYTESTREAM(
      "\"This is a long string that will overflow the small "
      "sixty-four character buffer of the lexer but hopefully we can recover\"\n"
      "medium_identifier_is_fine();\n"
      "this_is_a_long_identifier_name_that_will_also_overflow_the_sixty_four_character_buffer_"
      "because_we_never_learned_to_be_succinct_when_naming_things();"
      );

  gen_lexer_initialize(&lexer, &s);

  EXPECT( GENLEX_ERR_BUFFER_OVERFLOW, gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_ID_TOKEN, gen_lexer_next_token(&lexer) );
  EXPECT_STR( "medium_identifier_is_fine", gen_lexer_token_string(&lexer,NULL) );

  EXPECT( '(', gen_lexer_next_token(&lexer) );
  EXPECT( ')', gen_lexer_next_token(&lexer) );
  EXPECT( ';', gen_lexer_next_token(&lexer) );

  EXPECT( GENLEX_ERR_BUFFER_OVERFLOW, gen_lexer_next_token(&lexer) );

  EXPECT( '(', gen_lexer_next_token(&lexer) );
  EXPECT( ')', gen_lexer_next_token(&lexer) );
  EXPECT( ';', gen_lexer_next_token(&lexer) );

  EXPECT( 0, gen_lexer_next_token(&lexer) );
}

#define EXPECT_TOKEN_AT( tok, line, col, off, lexer ) \
  EXPECT( (tok) , gen_lexer_next_token((lexer)) ); \
  EXPECT( (off) , gen_lexer_token_off((lexer))  ); \
  EXPECT( (line), gen_lexer_token_line((lexer)) ); \
  EXPECT( (col) , gen_lexer_token_col((lexer))  )

DEFTEST( positions_are_correct )
{
  struct bytestream s = BYTESTREAM(
      " 32 + 15 - \n"
      " /* comment! */\n"
      "   5 * (3 + 2);\n"
      "foo(9);\n"
      "foo  (23) /* comment! */ + bar(9);\n"
      "x = \"bar\";\n");
  struct gen_lexer lexer;

  gen_lexer_initialize(&lexer, &s);

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 0, 1, 1, &lexer );
  EXPECT( 32, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( '+', 0, 4, 4, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 0, 6, 6, &lexer );
  EXPECT( 15, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( '-', 0, 9, 9, &lexer );

  EXPECT_TOKEN_AT( GENLEX_COMMENT_TOKEN, 1, 1, 13, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 2, 3, 31, &lexer );
  EXPECT( '5', s.bytes[31] );  /* check that offset is correct */
  EXPECT( 5, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( '*', 2, 5, 33, &lexer );
  EXPECT_TOKEN_AT( '(', 2, 7, 35, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 2, 8, 36, &lexer );
  EXPECT( 3, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( '+', 2, 10, 38, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 2, 12, 40, &lexer );
  EXPECT( '2', s.bytes[40] );  /* check that offset is correct */
  EXPECT( 2, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( ')', 2, 13, 41, &lexer );
  EXPECT_TOKEN_AT( ';', 2, 14, 42, &lexer );

  EXPECT_TOKEN_AT( GENLEX_ID_TOKEN, 3, 0, 44, &lexer );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  EXPECT_TOKEN_AT( '(', 3, 3, 47, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 3, 4, 48, &lexer );
  EXPECT( 9, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( ')', 3, 5, 49, &lexer );
  EXPECT_TOKEN_AT( ';', 3, 6, 50, &lexer );

  EXPECT_TOKEN_AT( GENLEX_ID_TOKEN, 4, 0, 52, &lexer );
  EXPECT_STR( "foo", gen_lexer_token_string(&lexer,NULL) );

  EXPECT_TOKEN_AT( '(', 4, 5, 57, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 4, 6, 58, &lexer );
  EXPECT( 23, gen_lexer_token_int_value(&lexer) );

  EXPECT_TOKEN_AT( ')', 4, 8, 60, &lexer );

  EXPECT_TOKEN_AT( GENLEX_COMMENT_TOKEN, 4, 10, 62, &lexer );

  EXPECT_TOKEN_AT( '+', 4, 25, 77, &lexer );
  EXPECT( '+', s.bytes[77] );  /* check that offset is correct */

  EXPECT_TOKEN_AT( GENLEX_ID_TOKEN, 4, 27, 79, &lexer );
  EXPECT_STR( "bar", gen_lexer_token_string(&lexer,NULL) );

  EXPECT_TOKEN_AT( '(', 4, 30, 82, &lexer );

  EXPECT_TOKEN_AT( GENLEX_INT_TOKEN, 4, 31, 83, &lexer );
  EXPECT( 9, gen_lexer_token_int_value(&lexer) );
  EXPECT( '9', s.bytes[83] );  /* check that offset is correct */

  EXPECT_TOKEN_AT( ')', 4, 32, 84, &lexer );
  EXPECT_TOKEN_AT( ';', 4, 33, 85, &lexer );

  EXPECT_TOKEN_AT( GENLEX_ID_TOKEN, 5, 0, 87, &lexer );
  EXPECT_STR( "x", gen_lexer_token_string(&lexer,NULL) );

  EXPECT_TOKEN_AT( '=', 5, 2, 89, &lexer );

  EXPECT_TOKEN_AT( GENLEX_STRING_TOKEN, 5, 4, 91, &lexer );
  EXPECT( '"', s.bytes[91] );  /* check that offset is correct */
  EXPECT_STR( "bar", gen_lexer_token_string(&lexer,NULL) );

  EXPECT_TOKEN_AT( ';', 5, 9, 96, &lexer );
  EXPECT( ';', s.bytes[96] );  /* check that offset is correct */

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

  RUNTEST( returns_literal_pairs );
  RUNTEST( returns_literals );
  RUNTEST( comments );

  RUNTEST( lexer_buffer_overflow_has_graceful_recovery );

  RUNTEST( positions_are_correct );
}


