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
** $-expansion, including the braced ${NAME} form.
**
** bash calls `${}` and `${1BAD}` a "bad substitution": it prints the whole
** word, sets the status to 1 and does NOT run the command. An unterminated
** `${` is a parse error (status 2), like an unclosed quote. All three
** return NULL, which every caller propagates, so the line never executes.
*/

static void	bad_subst(const char *word, t_shell *sh)
{
	char	*shown;

	shown = strip_quotes(word);
	put_str(2, "minishell: ");
	if (shown)
		put_str(2, shown);
	put_str(2, ": bad substitution\n");
	free(shown);
	sh->last_status = 1;
}

/*
** `$?`, `$$` and the positional `$0`…`$9`. Without braces only one digit
** is consumed, so `$1230` is `$1` + `230`; `$0` is the shell name.
** Returns NULL when s[*i] is none of these — that is not an error.
*/
static char	*expand_special(const char *s, int *i, t_shell *sh)
{
	if (s[*i] == '?')
		return ((*i)++, ms_itoa(sh->last_status));
	if (s[*i] == '$')
		return ((*i)++, ms_itoa(sh->pid));
	if (s[*i] == '0')
		return ((*i)++, ms_strdup("minishell"));
	if (s[*i] >= '1' && s[*i] <= '9')
		return ((*i)++, ms_strdup(""));
	return (NULL);
}

/* Takes ownership of `name`; `word` is the whole token, for the message. */
static char	*name_value(char *name, const char *word, t_shell *sh)
{
	char	*val;

	if (!is_identifier(name))
		return (bad_subst(word, sh), free(name), NULL);
	val = env_get(sh, name);
	free(name);
	if (!val)
		return (ms_strdup(""));
	return (ms_strdup(val));
}

/* On entry s[*i] == '{'. Consumes up to and including the closing '}'. */
static char	*expand_braced(const char *s, int *i, t_shell *sh)
{
	int		start;
	int		j;
	char	*name;
	char	*val;

	(*i)++;
	start = *i;
	while (s[*i] && s[*i] != '}')
		(*i)++;
	if (!s[*i])
	{
		put_str(2, "minishell: unexpected EOF while looking for"
			" matching `}'\n");
		sh->last_status = 2;
		return (NULL);
	}
	name = ms_substr(s, start, *i - start);
	(*i)++;
	if (!name)
		return (NULL);
	j = 0;
	val = expand_special(name, &j, sh);
	if (val && !name[j])
		return (free(name), val);
	return (free(val), name_value(name, s, sh));
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
