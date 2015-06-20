#ifndef GLEX_H
#define GLEX_H

/* GenLexer: General lexer for parsing things with C-like strings and
 * escape characters, numbers (integers and, optionally, reals),
 * keywords and identifiers, and single literal characters.
 *
 * This covers many C-like languages, domain-specific languages, and
 * markup languages, like JSON and TOML, that all basically have the
 * same definitions of whitespace, strings, numbers, and keywords.
 */

/* Required I/O definitions:
 *
 * GENLEX_IO_T
 *
 *      Type for IO context used in GenLex IO macros.  Defaults to void
 *      if not defined.
 *
 * ch = GENLEX_GETC(ctx)
 *
 *      Returns an int that is either the next byte of input (as an
 *      unsigned char) or -1 to indicate the end of the input stream.
 *
 * GENLEX_UNGETC(ch,ctx)
 *
 *      Pushes an unsigned char back onto the input.  ch will be the
 *      last character returned by GENLEX_GETC().
 *
 * Required configuration options:
 *
 * GENLEX_IS_SYMBOL(ch,pos)
 *
 *      Macro that checks if the character is part of a symbol or not.
 *      GENLEX_IS_SYMBOL(ch,pos) checks the character at position pos in a
 *      symbol is considered part of a symbol.  pos starts at zero for
 *      the first character in a symbol.  The macro should return a
 *      non-zero number if the character is allowed to be part of a
 *      symbol and zero if not.
 *
 *      The pos argument is mostly used to allow numerical digits (0-9)
 *      at the second and subsequent positions, but disallow them at the
 *      first position.
 *
 * GENLEX_LITERALS
 *
 *      A string of characters that should be returned as literal
 *      tokens: "(){}[],;" returns each of these characters as a literal
 *      token.
 *
 * GENLEX_LITERAL_PAIRS         (NOT IMPLEMENTED)
 *
 *      A list of literal pairs: two symbols that should be returned as
 *      a literal token.  If defined, these are evaluated before
 *      GENLEX_LITERALS.
 *
 *      The purpose of this is to allow ">=" and "==" to be returned as
 *      single tokens while allowing '>' and '=' to be in the literal
 *      string.
 *
 * GENLEX_STRING_MAX
 *
 *      Defines the maximum size of a string in the lexer.  The lexer
 *      declares an internal buffer that is GENLEX_STRING_MAX+1 bytes
 *      long and uses this to store strings and symbol identifiers.
 *
 * GENLEX_KEYWORDS
 *
 *      A list of keywords and associated token values.
 *
 * GENLEX_STRING_TOKEN
 *
 *      Token value used to indicate a string
 *
 * GENLEX_ID_TOKEN
 *
 *      Token value used to indicate an identifier.  A bare word that is
 *      not a keyword is considered an identifier.
 *
 * GENLEX_INT_TOKEN
 *
 *      Token value used to indicate an integer;
 *
 * GENLEX_INT_T
 *
 *      Integer type used in lexer.  If not defined, defaults to int.
 *
 *
 * Optional configuration options:
 *
 * GENLEX_COMMENT_PAIRS
 *
 *   A list of strings that delimit comments.  Examples are shown below.
 *   If not comment pairs are specified, then no comments are parsed.
 *   Comments are returned as whole strings by the lexer, and can either
 *   be used or discarded.
 *
 * GENLEX_IS_WHITESPACE(ch)
 *
 *      Must return a non-zero number if the character is a whitespace
 *      character and zero otherwise.  If this macro is not defined,
 *      whitespace defaults to the usual set.
 *
 * GENLEX_KEYWORD_TRIE          (NOT IMPLEMENTED)
 *
 *      #define to 1 to use a trie table for keyword lookup
 *
 * GENLEX_CONFIG_OCTAL          (NOT IMPLEMENTED)
 *
 *      #define to 1 to enable parsing octal numbers.
 *
 * GENLEX_CONFIG_HEXADECIMAL    (NOT IMPLEMENTED)
 *
 *      #define to 1 to enable parsing hexadecimal numbers.
 *
 * GENLEX_CONFIG_C_SUFFIX       (NOT IMPLEMENTED)
 *
 *      #define to 1 to enable parsing C-style integer and float
 *      suffixes (f, L, U, UL, etc.)
 *
 * GENLEX_CONFIG_FLOATS
 *
 *      #define to 1 to enable parsing floating point numbers.
 *
 * GENLEX_CONFIG_MULTILINE_STRING               (NOT IMPLEMENTED)
 *
 *      #define to 1 to enable parsing multi-line strings.
 *
 * GENLEX_CONFIG_SINGLE_QUOTE_STRING            (NOT IMPLEMENTED)
 *
 *      #define to 1 to enable parsing single-quote strings.  Otherwise,
 *      single-quotes assume a single character or a single escape
 *      character sequence.
 *
 * GENLEX_CONFIG_TRIPLE_QUOTED_STRING           (NOT IMPLEMENTED)
 *
 *      #define to 1 to enable parsing triple-quoted multi-line strings
 *
 * GENLEX_COMMENT_TOKEN
 *
 *      Token returned by lexer to indicate a comment. Required if
 *      GENLEX_COMMENT_PAIRS is defined.
 *
 * GENLEX_FLOAT_TOKEN
 *
 *      Token returned by lexer to indicate a floating point number.
 *      Required if GENLEX_CONFIG_REALS is defined to a non-zero value.
 *
 * GENLEX_FLOAT_T
 *
 *      Real (floating point) type used in lexer.  If not defined,
 *      defaults to double.
 *
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if !defined(GENLEX_IO_T)
#  define GENLEX_IO_T  void *
#endif

#if !defined(GENLEX_GETC) || !defined(GENLEX_UNGETC)
#  error  GENLEX_GETC and GENLEX_UNGETC must be defined
#endif

#if !defined(GENLEX_IS_SYMBOL)
#  error GENLEX_IS_SYMBOL must be defined
#endif

#if !defined(GENLEX_LITERALS)
#  error GENLEX_LITERALS must be defined
#endif

#if !defined(GENLEX_STRING_MAX)
#  error GENLEX_STRING_MAX must be defined
#endif

#if !defined(GENLEX_ID_TOKEN)
#  error GENLEX_ID_TOKEN must be defined
#endif

#if !defined(GENLEX_STRING_TOKEN)
#  error GENLEX_STRING_TOKEN must be defined
#endif

#if !defined(GENLEX_INT_TOKEN)
#  error GENLEX_INT_TOKEN must be defined
#endif

#if !defined(GENLEX_KEYWORDS)
#  error GENLEX_KEYWORDS must be defined
#endif

#if defined(GENLEX_COMMENT_PAIRS) && !defined(GENLEX_COMMENT_TOKEN)
#  error GENLEX_COMMENT_TOKEN must be defined when GENLEX_COMMENT_PAIRS is defined
#endif 

#if GENLEX_CONFIG_FLOATS
#  if !defined(GENLEX_FLOAT_T)
#    define GENLEX_FLOAT_T double
#  endif
#  if !defined(GENLEX_FLOAT_TOKEN)
#    error GENLEX_FLOAT_TOKEN must be defined
#  endif
#endif

/* Default configuration options if not already defined */
#if !defined(GENLEX_INT_T)
#  define GENLEX_INT_T int
#endif

#if !defined(GENLEX_IS_WHITESPACE)
#  include <ctype.h>
#  define GENLEX_IS_WHITESPACE(ch) (isspace(ch))
#endif

#define GENLEX_NUM_LITERALS  (sizeof(GENLEX_LITERALS)-1)

enum gen_lexer_errors {
  GENLEX_ERR_INVALID_CHAR        = -1,
  GENLEX_ERR_BUFFER_OVERFLOW     = -2,
  GENLEX_ERR_UNEXPECTED_EOF      = -3,
  GENLEX_ERR_UNEXPECTED_EOL      = -4,
  GENLEX_ERR_UNRECOGNIZED_ESCAPE = -5,
  GENLEX_ERR_INTEGER_OVERFLOW    = -6,
  GENLEX_ERR_FLOAT_OVERFLOW      = -7,
  GENLEX_ERR_INVALID_INTEGER     = -8,
  GENLEX_ERR_UNKNOWN_ERROR     = -100,
  GENLEX_ERR_INVALID_STATE     = -101,
  GENLEX_ERR_UNIMPLEMENTED    = -1000,  /* FIXME: should be removed after development */
};

struct gen_lexer {
  GENLEX_IO_T ctx;
  size_t blen;
  unsigned char buf[GENLEX_STRING_MAX];
  union {
    GENLEX_INT_T i;
#if GENLEX_CONFIG_FLOATS
    GENLEX_FLOAT_T f;
#endif
  } tval;
};

struct gen_lexer_keyword {
  const char *keyword;
  int token;
};

struct gen_lexer_literal_pair {
  const unsigned char pair[2];
  int token;
};

/* Currently comment pairs are restricted to two character to keep our
 * look-ahead buffer only one character.  This handles many common
 * comment pairs.
 */
struct gen_lexer_comment_pairs {
  const unsigned char beg[2]; /* string that delimits the beginning of a comment */
  const unsigned char end[2]; /* string that delimits the end of a comment */
  /* TODO: optionally enable nested comments */
};

/* Some common comment pairs as examples: */

/* C90 pairs of (slash-star, star-slash) */
#define GENLEX_C90_COMMENTS { { "/*", "*/" } }

/* C99/C++ also includes (slash-slash, EOL) */
#define GENLEX_C99_COMMENTS { { "/*", "*/" }, { "//", "\n" } }

/* sh/ksh/bash use (pound,EOL) */
#define GENLEX_SH_COMMENTS  { { "#", "\n" } }

static const unsigned char gen_lexer_literals[GENLEX_NUM_LITERALS] = GENLEX_LITERALS;
static const struct gen_lexer_keyword gen_lexer_keywords[] = GENLEX_KEYWORDS;

#if defined(GENLEX_COMMENT_PAIRS)
static const struct gen_lexer_comment_pairs gen_lexer_comments[] = GENLEX_COMMENT_PAIRS;
#define GENLEX_NUM_COMMENT_PAIRS  (sizeof(gen_lexer_comments)/sizeof(gen_lexer_comments[0]))
#endif

#if defined(GENLEX_LITERAL_PAIRS)
static const struct gen_lexer_literal_pair gen_lexer_literal_pairs[] = GENLEX_LITERAL_PAIRS;
#define GENLEX_NUM_LITERAL_PAIRS  (sizeof(gen_lexer_literal_pairs)/sizeof(gen_lexer_literal_pairs[0]))
#endif

#define GENLEX_NUM_KEYWORDS  (sizeof(gen_lexer_keywords)/sizeof(gen_lexer_keywords[0]))

/* Initializes the lexer structure with the IO context */
static int gen_lexer_initialize(struct gen_lexer *lexer, GENLEX_IO_T ctx);

/* Returns the next token, 0 at the end of the stream, or -1 if an error
 * occurred
 */
static int gen_lexer_next_token(struct gen_lexer *lexer);

static const unsigned char *gen_lexer_token_string(struct gen_lexer *lexer, size_t *lenp);
static GENLEX_INT_T gen_lexer_token_int_value(struct gen_lexer *lexer);

#if GENLEX_CONFIG_FLOATS
static GENLEX_FLOAT_T gen_lexer_token_float_value(struct gen_lexer *lexer);
#endif


/* Implementation */

static int genlex_skip_ws(struct gen_lexer *lexer)
{
  int c;

  while (c = GENLEX_GETC(lexer->ctx),
      (c != EOF) && GENLEX_IS_WHITESPACE(c)) {
    continue;
  }

  return c;
}

static int gen_lexer_initialize(struct gen_lexer *lexer, GENLEX_IO_T ctx)
{
  memset(lexer, 0, sizeof(*lexer));
  lexer->ctx = ctx;
  return 1;
}

static int gen_lexer_buf_add(struct gen_lexer *lexer, int ch)
{
  if (lexer->blen+1 >= sizeof(lexer->buf)) {
    return 0;
  }

  lexer->buf[lexer->blen++] = ch;
  return 1;
}

/* Shortcut macro that includes return-on-error behavior.
 *
 * This is mainly meant to do the default error handling that's almost
 * always done when using gen_lexer_buf_add
 */
#define GENLEXER_BUF_ADD(lx, c) do { \
  if (!gen_lexer_buf_add((lx),(c))) { \
    return GENLEX_ERR_BUFFER_OVERFLOW; \
  } } while(0)

static int gen_lexer_next_char_escaped(struct gen_lexer *lexer)
{
  int c = GENLEX_GETC(lexer->ctx);

  if (c == EOF) {
    return GENLEX_ERR_UNEXPECTED_EOF;
  }

  /* FIXME: escape sequences should be somewhat configurable */
  /* FIXME: this doesn't cover all of the ones recognized by C90 */
  switch (c) {
    case 'n': c = '\n'; break;
    case 't': c = '\t'; break;
    case 'r': c = '\r'; break;
    case '"': c = '"'; break;
    case '\\': break;
    default: return GENLEX_ERR_UNRECOGNIZED_ESCAPE;
  }

  return c;
}

static int gen_lexer_read_string(struct gen_lexer *lexer)
{
  /* TODO: add optional three-quote string support */
  /* TODO: add optional single-quote string support */
  int err = 0;

  for(;;) {
    int c = GENLEX_GETC(lexer->ctx);

    if (c == EOF) {
      if (!err) { err = GENLEX_ERR_UNEXPECTED_EOF; }
      return err;
    }

    /* TODO: add optional multiline string support */
    if (c == '\n') {
      if (!err) { err = GENLEX_ERR_UNEXPECTED_EOL; }
      return err;
    }

    if (c == '"') {
      if (!err) { return GENLEX_STRING_TOKEN; }
      return err;
    }

    if (c == '\\') {
      c = gen_lexer_next_char_escaped(lexer);
      if (c < 0) { /* error code */
        if (!err) { err = c; }
        return err;
      }
    }

    if (!gen_lexer_buf_add(lexer,c)) {
      if (!err) { err = GENLEX_ERR_BUFFER_OVERFLOW; }
      /* continue processing to consume string, if possible */
    }
  }
}

static int gen_lexer_read_char(struct gen_lexer *lexer)
{
  int c;

  c = GENLEX_GETC(lexer->ctx);
  if (c == '\\') {
    c = gen_lexer_next_char_escaped(lexer);
    if (c < 0) { return c; } /* error code */
  }

  GENLEXER_BUF_ADD(lexer,c);

  lexer->tval.i = c;

  c = GENLEX_GETC(lexer->ctx);
  if (c == EOF) {
    return GENLEX_ERR_UNEXPECTED_EOF;
  }

  if (c != '\'') {
    return GENLEX_ERR_INVALID_CHAR;
  }

  return GENLEX_INT_TOKEN;
}

static int gen_lexer_read_num(struct gen_lexer *lexer, int c)
{
  /* TODO: optional C99 intmax_t support */
  long value;
  GENLEX_INT_T lval;
  const char *s;
  char *end;
  int isfloat;

  /* TODO: optional hexidecimal and octal support */

  isfloat = 0;
  do {
    GENLEXER_BUF_ADD(lexer,c);

    c = GENLEX_GETC(lexer->ctx);
  } while (isnumber(c));

#if GENLEX_CONFIG_FLOATS
  /* check for decimal */
  if (c == '.') {
    isfloat = 1;
    do {
      GENLEXER_BUF_ADD(lexer,c);
      c = GENLEX_GETC(lexer->ctx);
    } while (isnumber(c));
  }

  /* check for exponent */
  if ((c == 'e') || (c=='E')) {
    isfloat = 1;
    GENLEXER_BUF_ADD(lexer,c);
    c = GENLEX_GETC(lexer->ctx);
    if ((c == '-') || (c == '+')) {
      GENLEXER_BUF_ADD(lexer,c);
      c = GENLEX_GETC(lexer->ctx);
    }

    if (!isnumber(c)) {
      return GENLEX_ERR_INVALID_CHAR;
    }

    do {
      GENLEXER_BUF_ADD(lexer,c);
      c = GENLEX_GETC(lexer->ctx);
    } while (isnumber(c));
  }
#endif

  if (c != EOF) {
    if (GENLEX_IS_SYMBOL(c,0)) {
      return GENLEX_ERR_INVALID_CHAR;
    }

    GENLEX_UNGETC(c,lexer->ctx);
  }

  errno = 0;
  s = (const char*)gen_lexer_token_string(lexer,NULL);

  if (isfloat) {
#if GENLEX_CONFIG_FLOATS
    GENLEX_FLOAT_T fvalue;

#if   GENLEX_FLOAT_T == float
    fvalue = strtof(s, &end);
#elif GENLEX_FLOAT_T == double
    fvalue = strtod(s, &end);
#else
    /* unknown, so default to long double */
    fvalue = strtold(s, &end);
#endif
  /* The lexer only invokes this routine when there's numeric input, so
   * *s should never be '\0', and we can just check if *end is not '\0'
   */
  if (*end != '\0') {
    return GENLEX_ERR_INVALID_INTEGER;
  }

  if (errno == ERANGE) {
    return GENLEX_ERR_FLOAT_OVERFLOW;
  }

  if (errno != 0) {
    return GENLEX_ERR_UNKNOWN_ERROR;
  }

  lexer->tval.f = fvalue;

  return GENLEX_FLOAT_TOKEN;

#else
  /* this should never be reached! */
  return GENLEX_ERR_INVALID_STATE;
#endif /* GENLEX_CONFIG_FLOATS */
  }

  /* TODO: optional suffix type support */

  /* FIXME: use a fixed base 10 for now since we don't officially
   * support hex or octal
   */
  /* use gen_lexer_token_string() because it null-terminates the buffer */
  value = strtol(s, &end, 10);

  /* The lexer only invokes this routine when there's numeric input, so
   * *s should never be '\0', and we can just check if *end is not '\0'
   */
  if (*end != '\0') {
    return GENLEX_ERR_INVALID_INTEGER;
  }

  if ((value == 0) && (errno != 0)) {
    if (errno == ERANGE) {
      return GENLEX_ERR_INTEGER_OVERFLOW;
    }

    if (errno == EINVAL) {
      return GENLEX_ERR_INVALID_INTEGER;
    }

    return GENLEX_ERR_UNKNOWN_ERROR;
  }

  lval = value;

  /* check that value fits in bounds of GENLEX_INT_T */
  if ((long)lval != value) {
    return GENLEX_ERR_INTEGER_OVERFLOW;
  }

  lexer->tval.i = lval;

  return GENLEX_INT_TOKEN;
}

static int gen_lexer_lookup_keyword(struct gen_lexer *lexer)
{
  int i;
  for (i = 0; i < GENLEX_NUM_KEYWORDS; i++) {
    if (strncmp((const char*)lexer->buf, gen_lexer_keywords[i].keyword, lexer->blen) == 0) {
      return gen_lexer_keywords[i].token;
    }
  }

  return -1;
}

static int gen_lexer_read_symbol(struct gen_lexer *lexer, int c)
{
  int tok;

  tok = 0;
  do {
    if (tok == 0) {
      if (!gen_lexer_buf_add(lexer, c)) {
        tok = GENLEX_ERR_BUFFER_OVERFLOW;
      }
    }

    c = GENLEX_GETC(lexer->ctx);
  } while (GENLEX_IS_SYMBOL(c, lexer->blen));

  if (c != EOF) {
    GENLEX_UNGETC(c, lexer->ctx);
  }

  if (tok != 0) {
    return tok; /* error code */
  }

  /* TODO: implement optional trie-table search */

  tok = gen_lexer_lookup_keyword(lexer);
  if (tok < 0) { tok = GENLEX_ID_TOKEN; }
  return tok;
}

#if defined(GENLEX_COMMENT_PAIRS)
static int gen_lexer_read_comment(struct gen_lexer *lexer, const unsigned char end[2])
{

  for(;;) {
    int c, next;
    c = GENLEX_GETC(lexer->ctx);

    if (c == EOF) {
      return GENLEX_ERR_UNEXPECTED_EOF;
    }

    if (c != end[0]) {
      GENLEXER_BUF_ADD(lexer,c);
      continue;
    }

    if (!end[1]) {
      return GENLEX_COMMENT_TOKEN;
    }

    next = GENLEX_GETC(lexer->ctx);
    if (next == end[1]) {
      return GENLEX_COMMENT_TOKEN;
    }

    GENLEXER_BUF_ADD(lexer,c);
    GENLEXER_BUF_ADD(lexer,next);
  }
}
#endif


static int gen_lexer_next_token(struct gen_lexer *lexer)
{
  int ch;
  const unsigned char *lit;

  lexer->blen = 0;
  ch = genlex_skip_ws(lexer);

  if (ch == EOF) { return 0; }

#if defined(GENLEX_COMMENT_PAIRS)
  {
    int i;
    for (i=0; i < GENLEX_NUM_COMMENT_PAIRS; i++) {
      int next;
      if (ch != gen_lexer_comments[i].beg[0]) { continue; }

      if (!gen_lexer_comments[i].beg[1]) {
        return gen_lexer_read_comment(lexer, gen_lexer_comments[i].end);
      }

      next = GENLEX_GETC(lexer->ctx);
      if (next != gen_lexer_comments[i].beg[1]) {
        GENLEX_UNGETC(next,lexer->ctx);
        continue;
      }

      return gen_lexer_read_comment(lexer, gen_lexer_comments[i].end);
    }
  }
#endif

  if (ch == '"') {
    return gen_lexer_read_string(lexer);
  }

  /* TODO: optional single-quote strings */
  if (ch == '\'') {
    return gen_lexer_read_char(lexer);
  }

  /* check for literals and literal pairs before numbers so '-' can be
   * returned as a literal symbol, or otherwised used to parse a number
   */
#if defined(GENLEX_LITERAL_PAIRS)
  {
    int i, c2;
    c2 = GENLEX_GETC(lexer->ctx);
    if (c2 != EOF) {
      for (i=0; i < GENLEX_NUM_LITERAL_PAIRS; i++) {
        if ((gen_lexer_literal_pairs[i].pair[0] == ch) &&
            (gen_lexer_literal_pairs[i].pair[1] == c2)) {
          GENLEXER_BUF_ADD(lexer,ch);
          GENLEXER_BUF_ADD(lexer,c2);
          return gen_lexer_literal_pairs[i].token;
        }
      }
      GENLEX_UNGETC(c2, lexer->ctx);
    }
  }
#endif

  lit = memchr(gen_lexer_literals, ch, sizeof(gen_lexer_literals));
  if (lit && *lit) { return ch; }

  if (isnumber(ch) || (ch == '-')) {
    return gen_lexer_read_num(lexer, ch);
  }

  if (GENLEX_IS_SYMBOL(ch, 0)) {
    /* identifier or keyword */
    return gen_lexer_read_symbol(lexer, ch);
  }

  return GENLEX_ERR_INVALID_CHAR;
}

static const unsigned char *gen_lexer_token_string(struct gen_lexer *lexer, size_t *lenp)
{
  if (lexer->blen < sizeof(lexer->buf)) {
    lexer->buf[lexer->blen] = '\0';
  }
  if (lenp) { *lenp = lexer->blen; }
  return lexer->buf;
}

static GENLEX_INT_T gen_lexer_token_int_value(struct gen_lexer *lexer)
{
  return lexer->tval.i;
}

#if GENLEX_CONFIG_FLOATS
static GENLEX_FLOAT_T gen_lexer_token_float_value(struct gen_lexer *lexer)
{
  return lexer->tval.f;
}
#endif

#endif /* GLEX_H */

