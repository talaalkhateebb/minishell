/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   expander_braced.c                                    :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** The braced form, `${NAME}`, and the two ways it can go wrong: a missing
** closing brace, and a name that is not an identifier.
**
** It lives apart from expander_dollar.c only because it does not fit there:
** spelling each branch out as its own statements — rather than returning a
** free() and a value together — needs more room than one file's worth of
** functions had left.
*/

/*
** bash's own wording for `${}` and `${1BAD}`: the whole word is echoed
** back, not just the offending name, and the status goes to 1 without the
** command being run.
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
** An unterminated `${` is a syntax error, and bash reports it with the
** status it gives every other one: 2.
*/
static char	*braced_eof(t_shell *sh)
{
	put_str(2, "minishell: unexpected EOF while looking for"
		" matching `}'\n");
	sh->last_status = 2;
	return (NULL);
}

/* Takes ownership of `name`; `word` is the whole token, for the message. */
static char	*name_value(char *name, const char *word, t_shell *sh)
{
	char	*val;

	if (!is_identifier(name))
	{
		bad_subst(word, sh);
		free(name);
		return (NULL);
	}
	val = env_get(sh, name);
	free(name);
	if (!val)
		return (ms_strdup(""));
	return (ms_strdup(val));
}

/* On entry s[*i] == '{'. Consumes up to and including the closing '}'. */
char	*expand_braced(const char *s, int *i, t_shell *sh)
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
		return (braced_eof(sh));
	name = ms_substr(s, start, *i - start);
	(*i)++;
	if (!name)
		return (NULL);
	j = 0;
	val = expand_special(name, &j, sh);
	if (val && !name[j])
	{
		free(name);
		return (val);
	}
	free(val);
	return (name_value(name, s, sh));
}
