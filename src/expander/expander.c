/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   expander.c                                           :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Expander — Person A (frontend).
**
** Consumes a raw token value with its quotes still attached:
**   - outside quotes and inside "..."  → $VAR and $? expand
**   - inside '...'                     → nothing expands
**   - quote characters are removed
**
** Unquoted expansions are field-split on IFS (space/tab/newline), so
** `FOO="a b"; echo $FOO` becomes two arguments. Quoted expansions are not.
*/

static int	is_ifs(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
}

/* Hands the finished field to argv and opens a fresh, unquoted one. */
static int	push_field(struct s_expand *exp)
{
	if (argv_append(exp->cmd, exp->cur))
		return (1);
	exp->quoted = 0;
	exp->cur = ms_strdup("");
	if (!exp->cur)
		return (1);
	return (0);
}

/*
** Append an unquoted expansion into the current field, splitting on IFS.
**
** exp->quoted tracks the field being built, not the whole token — that is
** the whole trick. A boundary flushes the field when it holds text OR when
** a quoted segment opened it, so `""$var` keeps its empty first field the
** way bash does, and clearing the flag stops those same quotes from also
** marking the NEXT field. Trailing IFS then leaves an empty unquoted field,
** which expand_to_argv() drops — no phantom last argument.
*/
static int	append_split(struct s_expand *exp, const char *val)
{
	int	i;

	i = 0;
	while (val[i])
	{
		if (is_ifs(val[i]))
		{
			if ((exp->cur[0] || exp->quoted) && push_field(exp))
				return (1);
			while (val[i] && is_ifs(val[i]))
				i++;
		}
		else
			exp->cur = append_char(exp->cur, val[i++]);
		if (!exp->cur)
			return (1);
	}
	return (0);
}

static int	expand_dollar_branch(struct s_expand *exp, const char *s, int *i)
{
	char	*val;

	val = expand_dollar(s, i, exp->sh);
	if (!val || append_split(exp, val))
		return (free(val), free(exp->cur), 1);
	free(val);
	return (0);
}

static int	expand_loop(struct s_expand *exp, const char *s, int *i)
{
	while (s && s[*i])
	{
		if (s[*i] == '\'')
		{
			exp->quoted = 1;
			exp->cur = handle_single(s, i, exp->cur);
		}
		else if (s[*i] == '"')
		{
			exp->quoted = 1;
			exp->cur = handle_double(s, i, exp->cur, exp->sh);
		}
		else if (s[*i] == '$' && s[*i + 1])
		{
			if (expand_dollar_branch(exp, s, i))
				return (1);
		}
		else
			exp->cur = append_char(exp->cur, s[(*i)++]);
		if (!exp->cur)
			return (1);
	}
	return (0);
}

/*
** Expand one token into zero or more argv entries. Empty unquoted results
** disappear; an explicitly quoted empty string (`""`) is kept.
*/
int	expand_to_argv(t_cmd *cmd, const char *s, t_shell *sh)
{
	struct s_expand	exp;
	int				i;

	exp.cmd = cmd;
	exp.sh = sh;
	exp.quoted = 0;
	exp.cur = tilde_seed(s, &i, sh);
	if (!exp.cur)
		return (1);
	if (expand_loop(&exp, s, &i))
		return (1);
	if (exp.cur[0] || exp.quoted)
		return (argv_append(cmd, exp.cur));
	return (free(exp.cur), 0);
}
