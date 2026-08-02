/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   expander_dollar.c                                    :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** $-expansion. The braced ${NAME} form lives in expander_braced.c, which
** also holds bash's "bad substitution" and unterminated-`${` errors: all
** of those return NULL, which every caller propagates, so the line never
** executes.
*/

/*
** `$?`, `$$` and the positional `$0`…`$9`. Without braces only one digit
** is consumed, so `$1230` is `$1` + `230`; `$0` is the shell name.
** Returns NULL when s[*i] is none of these — that is not an error, and
** *i must not move in that case, so the character is classified first and
** consumed only once it is known to be one of ours.
*/
char	*expand_special(const char *s, int *i, t_shell *sh)
{
	char	c;

	c = s[*i];
	if (c != '?' && c != '$' && (c < '0' || c > '9'))
		return (NULL);
	(*i)++;
	if (c == '?')
		return (ms_itoa(sh->last_status));
	if (c == '$')
		return (ms_itoa(sh->pid));
	if (c == '0')
		return (ms_strdup("minishell"));
	return (ms_strdup(""));
}

/*
** On entry s[*i] == '$' and s[*i + 1] is not '\0'. Consumes the whole
** $-expression and returns its value as a fresh string. A `$` followed by
** anything that cannot start a name is left as a literal `$`.
*/
char	*expand_dollar(const char *s, int *i, t_shell *sh)
{
	int		start;
	char	*name;
	char	*val;

	(*i)++;
	if (s[*i] == '{')
		return (expand_braced(s, i, sh));
	val = expand_special(s, i, sh);
	if (val)
		return (val);
	if (!is_var_start(s[*i]))
		return (ms_strdup("$"));
	start = *i;
	while (s[*i] && is_var_char(s[*i]))
		(*i)++;
	name = ms_substr(s, start, *i - start);
	if (!name)
		return (NULL);
	val = env_get(sh, name);
	free(name);
	if (!val)
		return (ms_strdup(""));
	return (ms_strdup(val));
}
