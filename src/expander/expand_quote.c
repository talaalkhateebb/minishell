#include "minishell.h"

/*
** ============================================================
** expand_quote.c — quote tracking and quote-only stripping.
**
** handle_quote_state — toggles a quote state when ' or " is seen.
** strip_quotes_only  — drops ' and " from a string without doing
**                      $ expansion (used for heredoc delimiters).
** delim_is_quoted    — does this heredoc delimiter contain any
**                      quote chars? If yes, the heredoc body is
**                      NOT expanded.
** ============================================================
*/

/*
** handle_quote_state — react to char c given current quote state *q.
**
** *q values:  0 = outside quotes
**             '\'' = inside single quotes (no $ expansion)
**             '"'  = inside double quotes ($ expansion allowed)
**
** Returns 1 if c is a quote character we should SKIP (do not emit).
** Returns 0 if c is a regular character the caller should handle.
**
** Single-quote inside double-quote (or vice-versa) is just a
** literal character — handled by the "matching only" rules.
*/
int	handle_quote_state(char c, char *q)
{
	if (!*q && (c == '\'' || c == '"'))
	{
		*q = c;
		return (1);
	}
	if (*q && c == *q)
	{
		*q = 0;
		return (1);
	}
	return (0);
}

/*
** strip_count — count the characters we WOULD keep.
**
** Walks the string in count-mode, tracking quotes. Every char
** that isn't a "skip me" quote contributes 1 to the length.
*/
static size_t	strip_count(const char *w)
{
	size_t	i;
	size_t	pos;
	char	q;

	i = 0;
	pos = 0;
	q = 0;
	while (w[i])
	{
		if (handle_quote_state(w[i], &q))
			i++;
		else
		{
			pos++;
			i++;
		}
	}
	return (pos);
}

/*
** strip_fill — second pass: write chars into 'out'.
*/
static void	strip_fill(const char *w, char *out)
{
	size_t	i;
	size_t	pos;
	char	q;

	i = 0;
	pos = 0;
	q = 0;
	while (w[i])
	{
		if (handle_quote_state(w[i], &q))
			i++;
		else
		{
			out[pos] = w[i];
			pos++;
			i++;
		}
	}
	out[pos] = '\0';
}

/*
** strip_quotes_only — return a fresh string with all matched
** quote chars removed and NO $ expansion.
**
** Used for heredoc delimiters: "EOF" → EOF, 'E'OF → EOF.
** Returns NULL on malloc failure.
*/
char	*strip_quotes_only(const char *w)
{
	size_t	len;
	char	*out;

	len = strip_count(w);
	out = malloc(len + 1);
	if (!out)
		return (NULL);
	strip_fill(w, out);
	return (out);
}

/*
** delim_is_quoted — does this heredoc delimiter contain a quote?
**
** If yes, Bash treats the heredoc body literally (no $ expansion).
** That answer is stored in r->expand_heredoc by expand_one_redir.
*/
int	delim_is_quoted(const char *d)
{
	size_t	i;

	i = 0;
	while (d[i])
	{
		if (d[i] == '\'' || d[i] == '"')
			return (1);
		i++;
	}
	return (0);
}
