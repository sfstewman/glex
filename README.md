# GenLexer: A generic, configurable lexer #

After writing a number of lexers by hand that all have the same basic needs,
I've tried to combine them into something that covers these needs in
a flexible, yet performant, way.  The lexers I've written have the following
similarities:

1. They must robustly parse strings, including strings with C-like escape
   characters. The strings are generally parsed in the same way in many lexers
   that need to parse strings with C-like escape characters, so this code can
   easily be shared between many lexers.

2. They must robustly parse integers.  Many must parse floating point numbers
   robustly and accurately, as well.

3. They must recognize literal characters like '(', ')', ';', etc. and return
   them in a format that's easy for tools like Yacc to recognize and use.

4. They must recognize bare words: identifiers and keywords.  Keywords must be
   recognized and converted into appropriate tokens.


