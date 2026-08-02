/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   expander_quotes.c                                    :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*handle_single(const char *s, int *i, char *res)
{
	(*i)++;
	while (s[*i] && s[*i] != '\'')
	{
		res = append_char(res, s[*i]);
		(*i)++;
	}
	if (s[*i] == '\'')
		(*i)++;
	return (res);
}

char	*handle_double(const char *s, int *i, char *res, t_shell *sh)
{
	char	*val;

	(*i)++;
	while (s[*i] && s[*i] != '"' && res)
	{
		if (s[*i] == '$' && s[*i + 1])
		{
			val = expand_dollar(s, i, sh);
			if (!val)
				return (free(res), NULL);
			res = append_str(res, val);
			free(val);
		}
		else
		{
			res = append_char(res, s[*i]);
			(*i)++;
		}
	}
	if (res && s[*i] == '"')
		(*i)++;
	return (res);
}

char	*expand_word(const char *s, t_shell *sh)
{
	char	*res;
	char	*val;
	int		i;

	res = tilde_seed(s, &i, sh);
	while (s && s[i] && res)
	{
		if (s[i] == '\'')
			res = handle_single(s, &i, res);
		else if (s[i] == '"')
			res = handle_double(s, &i, res, sh);
		else if (s[i] == '$' && s[i + 1])
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

/*
** Quote removal with no expansion — used for heredoc delimiters, where
** << "EOF" and << 'EOF' both mean the delimiter EOF with expansion off.
*/
char	*strip_quotes(const char *s)
{
	char	*res;
	int		i;

	res = ms_strdup("");
	i = 0;
	while (s && s[i])
	{
		if (s[i] == '\'')
			res = handle_single(s, &i, res);
		else if (s[i] == '"')
		{
			i++;
			while (s[i] && s[i] != '"')
				res = append_char(res, s[i++]);
			if (s[i] == '"')
				i++;
		}
		else
			res = append_char(res, s[i++]);
	}
	return (res);
}

int	is_quoted(const char *s)
{
	int	i;

	i = 0;
	while (s && s[i])
	{
		if (s[i] == '\'' || s[i] == '"')
			return (1);
		i++;
	}
	return (0);
}
