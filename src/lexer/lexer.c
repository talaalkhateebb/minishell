#include "minishell.h"

/*
** ============================================================
** lexer.c — entry point of the lexer.
**
** The lexer cuts the raw input line into a linked list of
** tokens (t_token). Each token is either:
**   - T_WORD                : a command name, argument, filename
**   - T_PIPE                : |
**   - T_REDIR_IN / _OUT     : < / >
**   - T_HEREDOC / T_APPEND  : << / >>
**
** Quote characters ('...' and "...") are KEPT inside word tokens.
** The expander strips them later — that's also where $VAR is
** resolved, so the expander needs to know which segments were
** quoted (and which kind).
**
** Tokens are produced left-to-right, joined as a singly linked
** list. On any syntax error (unclosed quote, malloc fail) we
** free everything we built and return NULL.
** ============================================================
*/

/*
** is_space — POSIX whitespace check (no <ctype.h> dependency, since
** isspace() isn't on the allowed-function list).
*/
int	is_space(char c)
{
	return (c == ' ' || c == '\t' || c == '\n'
		|| c == '\v' || c == '\f' || c == '\r');
}

/*
** is_op_char — does this char start an operator? (| < >)
** & is intentionally NOT here: minishell mandatory doesn't support
** &&, ||, or background jobs. If a user types &, it'll be picked
** up as part of a word token, and execve will reject it.
*/
int	is_op_char(char c)
{
	return (c == '|' || c == '<' || c == '>');
}

/*
** check_unclosed_quote — pre-pass that scans the WHOLE line once
** to detect an unclosed quote.
**
** Walks character by character. 'q' tracks the current quote
** state: 0 = outside quotes, '\'' = inside single quotes, '"' =
** inside double quotes. We toggle state when we see a matching
** quote. At end-of-string, if q is still non-zero, a quote was
** never closed → syntax error.
**
** Bash prints "syntax error: unexpected end of file" for this.
** We print a clear message and let tokenize() return NULL.
*/
static int	check_unclosed_quote(const char *line)
{
	size_t	i;
	char	q;

	q = 0;
	i = 0;
	while (line[i])
	{
		if (!q && (line[i] == '\'' || line[i] == '"'))
			q = line[i];
		else if (q && line[i] == q)
			q = 0;
		i++;
	}
	return (q != 0);
}

/*
** lex_step — one iteration of the main loop.
**
** Skips whitespace, then dispatches: if the next char is an
** operator char, call handle_op; otherwise call handle_word.
** Returns 0 on success, non-zero on error.
*/
static int	lex_step(const char *line, size_t *i, t_token **head)
{
	while (line[*i] && is_space(line[*i]))
		(*i)++;
	if (!line[*i])
		return (0);
	if (is_op_char(line[*i]))
		return (handle_op(line, i, head));
	return (handle_word(line, i, head));
}

/*
** tokenize — public entry. Input string → linked list of tokens.
**
** Returns NULL on:
**   - unclosed quote (writes error to stderr)
**   - malloc failure in any sub-call
**
** On error, anything partially built is freed before returning.
** On success, returns the head of the token list — caller frees
** with free_tokens().
*/
t_token	*tokenize(const char *line)
{
	t_token	*head;
	size_t	i;

	if (check_unclosed_quote(line))
	{
		write(2, "minishell: syntax error: unclosed quote\n", 40);
		return (NULL);
	}
	head = NULL;
	i = 0;
	while (line[i])
	{
		if (lex_step(line, &i, &head) != 0)
		{
			free_tokens(head);
			return (NULL);
		}
	}
	return (head);
}
