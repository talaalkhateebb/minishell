/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   expander_utils.c                                     :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/* Appends `add` to `res`, freeing `res`. Returns the new string. */
char	*append_str(char *res, const char *add)
{
	char	*out;
	size_t	i;
	size_t	j;

	if (!res)
		return (NULL);
	if (!add)
		return (res);
	out = malloc(ms_strlen(res) + ms_strlen(add) + 1);
	if (!out)
		return (free(res), NULL);
	i = 0;
	while (res[i])
	{
		out[i] = res[i];
		i++;
	}
	j = 0;
	while (add[j])
		out[i++] = add[j++];
	out[i] = '\0';
	return (free(res), out);
}

char	*append_char(char *res, char c)
{
	char	buf[2];

	buf[0] = c;
	buf[1] = '\0';
	return (append_str(res, buf));
}

/*
** On entry s[*i] == '$'. Consumes the whole $-expression and returns its
** value as a fresh string. `$?` becomes the last exit status, `$NAME`
** becomes the env value (empty when unset), and a `$` followed by
** anything else is left as a literal `$`.
*/
char	*expand_dollar(const char *s, int *i, t_shell *sh)
{
	int		start;
	char	*name;
	char	*val;

	(*i)++;
	if (s[*i] == '?')
		return ((*i)++, ms_itoa(sh->last_status));
	/*
	** bash: `$0`…`$9` are positional. Only one digit is consumed without
	** braces, so `$1230` is `$1` + `230`. `$0` is the shell name.
	*/
	if (s[*i] >= '0' && s[*i] <= '9')
	{
		if (s[*i] == '0')
			return ((*i)++, ms_strdup("minishell"));
		return ((*i)++, ms_strdup(""));
	}
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

/*
** Heredoc body line: $ expands, but quotes are NOT special here — a
** literal " inside a heredoc stays a literal ".
*/
char	*expand_heredoc_line(const char *s, t_shell *sh)
{
	char	*res;
	char	*val;
	int		i;

	res = ms_strdup("");
	i = 0;
	while (s && s[i])
	{
		if (s[i] == '$' && s[i + 1])
		{
			val = expand_dollar(s, &i, sh);
			res = append_str(res, val);
			free(val);
		}
		else
			res = append_char(res, s[i++]);
	}
	return (res);
}
