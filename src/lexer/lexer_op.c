#include "minishell.h"

/*
** ============================================================
** lexer_op.c — reading operator tokens (| < > << >>).
**
** Operators are simple: one or two characters, no quoting, no
** ambiguity. We just decide the type and the width, then slice
** that many characters out of the line.
**
** | is always width 1.
** < and > peek at the next char to decide width 1 vs 2.
** ============================================================
*/

/*
** op_type — figure out which operator the current char starts.
**
** Peeks at line[i] and line[i+1]. For < and >, if the next char
** is the same we have the doubled form (<< or >>). Otherwise
** single (< or >). Pipe is always a single |.
**
** Reading line[i+1] is safe even at end-of-string: C strings are
** null-terminated, so line[i+1] is at worst '\0' (which we just
** don't match against < or >).
*/
static t_token_type	op_type(const char *line, size_t i)
{
	if (line[i] == '|')
		return (T_PIPE);
	if (line[i] == '<' && line[i + 1] == '<')
		return (T_HEREDOC);
	if (line[i] == '<')
		return (T_REDIR_IN);
	if (line[i] == '>' && line[i + 1] == '>')
		return (T_APPEND);
	return (T_REDIR_OUT);
}

/*
** op_width — 1 or 2 characters wide?
**
** Returns 2 for the doubled forms (<< and >>), 1 otherwise.
*/
static int	op_width(const char *line, size_t i)
{
	if (line[i] == '<' && line[i + 1] == '<')
		return (2);
	if (line[i] == '>' && line[i + 1] == '>')
		return (2);
	return (1);
}

/*
** handle_op — extract the operator at *i and append a token.
**
** Calls extract_word to slice the operator characters (so the
** token value is literally "|", "<", ">", "<<", or ">>"). The
** stored value is rarely needed (the type carries the meaning),
** but keeping it makes debugging easier and lets the parser
** print exact error messages.
**
** Updates *i past the operator. Returns 0 on success, 1 on
** malloc failure.
*/
int	handle_op(const char *line, size_t *i, t_token **head)
{
	t_token_type	type;
	int				width;
	char			*value;
	t_token			*node;

	type = op_type(line, *i);
	width = op_width(line, *i);
	value = extract_word(line, *i, *i + width);
	if (!value)
		return (1);
	node = token_new(value, type);
	if (!node)
	{
		free(value);
		return (1);
	}
	token_append(head, node);
	*i += width;
	return (0);
}
