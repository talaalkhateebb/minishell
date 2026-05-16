#include "minishell.h"

/*
** Lexer — Person A (frontend), Days 3–5.
**
** Walk the input character by character and produce a linked list of
** t_token. Whitespace separates tokens (outside quotes). Operators are
** | < > << >>. Quoted segments stick to adjacent words.
**
** TODO:
**   - tokenize(const char *line)  → t_token *
**   - free_tokens(t_token *)
**   - detect unclosed quotes / illegal characters
**
** Trap: `echo "hello"world'!'` → ONE word `helloworld!` after expansion.
*/
