/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   expander_split.c                                     :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Field splitting for unquoted expansions — lifted out of expander.c so
** that neither file carries more functions than the Norm allows.
*/

static int	is_ifs(char c)
{
	return (c == ' ' || c == '\t' || c == '\n');
}

/* Hands the finished field to argv and opens a fresh, unquoted one. */
static int	push_field(struct s_expand *exp)
{
	if (argv_append(exp->cmd, exp->cur))
	{
		exp->cur = NULL;
		return (1);
	}
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
int	append_split(struct s_expand *exp, const char *val)
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
