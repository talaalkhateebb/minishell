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
** Starts the result string for a word, expanding a leading `~` to $HOME
** and setting *i past it. Only a bare `~` or a `~/...` prefix expands:
** `~user`, `a~b` and a quoted `"~"` stay literal, and so does `~` when
** HOME is unset — bash does the same in every one of those cases.
*/
char	*tilde_seed(const char *s, int *i, t_shell *sh)
{
	char	*home;

	*i = 0;
	if (!s || s[0] != '~' || (s[1] && s[1] != '/'))
		return (ms_strdup(""));
	home = env_get(sh, "HOME");
	if (!home)
		return (ms_strdup(""));
	*i = 1;
	return (ms_strdup(home));
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
	while (s && s[i] && res)
	{
		if (s[i] == '$' && s[i + 1])
		{
			val = expand_dollar(s, &i, sh);
			if (!val)
				return (free(res), NULL);
			res = append_str(res, val);
			free(val);
		}
		else
			res = append_char(res, s[i++]);
	}
	return (res);
}
