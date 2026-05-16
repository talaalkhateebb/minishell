#include "minishell.h"

/*
** ============================================================
** expand_word.c — turn one raw word into its expanded form.
**
** Two-pass strategy (no realloc available):
**   1. walk the input with a NULL dst, just counting output chars
**   2. malloc that many bytes + 1
**   3. walk again with dst set, actually writing
**
** Both passes call the same step() so they can't drift apart.
**
** Examples:
**   "hello $USER"   →   "hello tala"
**   '$USER'         →   "$USER"   (no expansion inside single)
**   "abc"'def'ghi   →   "abcdefghi"
**   $?              →   "0"       (or whatever last_status is)
** ============================================================
*/

/*
** emit_char — write one char to dst (or just count if dst==NULL).
*/
static void	emit_char(char c, t_xstate *st)
{
	if (st->dst)
		st->dst[st->pos] = c;
	st->pos++;
}

/*
** emit_str — write each char of s to dst (or count). Stops at \0.
*/
static void	emit_str(const char *s, t_xstate *st)
{
	size_t	i;

	if (!s)
		return ;
	i = 0;
	while (s[i])
	{
		emit_char(s[i], st);
		i++;
	}
}

/*
** handle_dollar — handle a $ that we already decided to expand.
**
** Caller has confirmed: not inside single quotes, and the char
** after $ is either '?' or a valid var-name start.
**
**   $?    → emit itoa_status(sh->last_status)
**   $VAR  → emit env value (or empty if unset)
**
** Advances *i past the consumed characters.
*/
static void	handle_dollar(const char *w, size_t *i, t_xstate *st)
{
	size_t	nlen;
	char	*value;

	(*i)++;
	if (w[*i] == '?')
	{
		value = itoa_status(st->sh->last_status);
		emit_str(value, st);
		free(value);
		(*i)++;
		return ;
	}
	nlen = var_name_len(&w[*i]);
	value = get_var_value(w, *i, nlen, st->sh);
	emit_str(value, st);
	free(value);
	*i += nlen;
}

/*
** step — process exactly one input position.
**
** Three cases:
**   1. Quote char that toggles state          → consume, no emit
**   2. $ followed by a valid expansion target → call handle_dollar
**   3. Anything else                          → emit the char as-is
*/
static void	step(const char *w, size_t *i, t_xstate *st)
{
	if (handle_quote_state(w[*i], &st->q))
	{
		(*i)++;
		return ;
	}
	if (st->q != '\'' && w[*i] == '$'
		&& (w[*i + 1] == '?' || is_var_start(w[*i + 1])))
	{
		handle_dollar(w, i, st);
		return ;
	}
	emit_char(w[*i], st);
	(*i)++;
}

/*
** expand_word — public entry. Returns a freshly malloc'd expanded
** version of w. Caller owns the result.
**
** Runs step() twice over the input: once to count (st.dst=NULL),
** once to fill. Resets all state between passes.
*/
char	*expand_word(const char *w, t_shell *sh)
{
	t_xstate	st;
	size_t		i;
	char		*out;

	st.dst = NULL;
	st.pos = 0;
	st.q = 0;
	st.sh = sh;
	i = 0;
	while (w[i])
		step(w, &i, &st);
	out = malloc(st.pos + 1);
	if (!out)
		return (NULL);
	st.dst = out;
	st.pos = 0;
	st.q = 0;
	i = 0;
	while (w[i])
		step(w, &i, &st);
	out[st.pos] = '\0';
	return (out);
}
