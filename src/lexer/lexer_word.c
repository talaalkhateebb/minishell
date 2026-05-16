#include "minishell.h"

/*
** ============================================================
** lexer_word.c — reading WORD tokens.
**
** A "word" in our lexer is anything that isn't whitespace and
** isn't an operator (| < >). Crucially, whitespace and operators
** INSIDE quotes do NOT end the word.
**
** Examples (one word each):
**   echo
**   "hello world"
**   'a|b<c>d'
**   "abc"'def'ghi    ← three glued segments, one word total
**
** The lexer doesn't try to remove the quote chars — it copies
** them as-is into the token's value. The expander later walks
** the value and handles quotes + $-expansion together.
** ============================================================
*/

/*
** skip_quoted — walk past a single- or double-quoted region.
**
** Called only after we've already confirmed (via check_unclosed_
** quote in lexer.c) that every quote has a matching close. So
** we trust that the closing quote exists.
**
** i points AT the opening quote. Returns the index AFTER the
** closing quote.
*/
static size_t	skip_quoted(const char *line, size_t i, char q)
{
	i++;
	while (line[i] && line[i] != q)
		i++;
	if (line[i])
		i++;
	return (i);
}

/*
** find_word_end — find the index where the current word ends.
**
** A word ends at the first whitespace or operator char that is
** NOT inside quotes. Returns the index of that boundary (which
** is one past the last char of the word).
**
** Example with line = "abc 'd e' fg":
**   start = 0 → walks through 'a','b','c','\'','d',' ','e','\'',
**   stops at the space after the closing quote → returns 9.
*/
static size_t	find_word_end(const char *line, size_t i)
{
	while (line[i] && !is_space(line[i]) && !is_op_char(line[i]))
	{
		if (line[i] == '\'')
			i = skip_quoted(line, i, '\'');
		else if (line[i] == '"')
			i = skip_quoted(line, i, '"');
		else
			i++;
	}
	return (i);
}

/*
** extract_word — copy line[start..end) into fresh malloc'd memory.
**
** Returns NULL on malloc failure. The returned string is the
** raw characters including any quote marks.
** Also used by handle_op (in lexer_op.c) to slice operator text.
*/
char	*extract_word(const char *line, size_t start, size_t end)
{
	size_t	len;
	char	*out;
	size_t	i;

	len = end - start;
	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	while (i < len)
	{
		out[i] = line[start + i];
		i++;
	}
	out[len] = '\0';
	return (out);
}

/*
** handle_word — extract the current word and append it as a token.
**
** Updates *i to point past the word. Returns 0 on success, 1 on
** malloc failure. If token_new fails we still free the value we
** just allocated — otherwise it would leak.
*/
int	handle_word(const char *line, size_t *i, t_token **head)
{
	size_t	start;
	size_t	end;
	char	*value;
	t_token	*node;

	start = *i;
	end = find_word_end(line, start);
	value = extract_word(line, start, end);
	if (!value)
		return (1);
	node = token_new(value, T_WORD);
	if (!node)
	{
		free(value);
		return (1);
	}
	token_append(head, node);
	*i = end;
	return (0);
}
