/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+#             */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr       */
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

/* Copies a '...' run verbatim. On entry s[*i] is the opening quote. */
static char	*handle_single(const char *s, int *i, char *res)
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

/* Copies a "..." run, expanding $ inside it (no field splitting). */
static char	*handle_double(const char *s, int *i, char *res, t_shell *sh)
{
	char	*val;

	(*i)++;
	while (s[*i] && s[*i] != '"')
	{
		if (s[*i] == '$' && s[*i + 1])
		{
			val = expand_dollar(s, i, sh);
			res = append_str(res, val);
			free(val);
		}
		else
		{
			res = append_char(res, s[*i]);
			(*i)++;
		}
	}
	if (s[*i] == '"')
		(*i)++;
	return (res);
}

char	*expand_word(const char *s, t_shell *sh)
{
	char	*res;
	char	*val;
	int		i;

	res = ms_strdup("");
	i = 0;
	while (s && s[i])
	{
		if (s[i] == '\'')
			res = handle_single(s, &i, res);
		else if (s[i] == '"')
			res = handle_double(s, &i, res, sh);
		else if (s[i] == '$' && s[i + 1])
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

/*
** Append an unquoted expansion into the current field, splitting on IFS.
** Completed fields are pushed to argv; trailing partial text stays in *cur.
*/
static int	append_split(t_cmd *cmd, char **cur, const char *val)
{
	int	i;

	i = 0;
	while (val[i])
	{
		if (is_ifs(val[i]))
		{
			if ((*cur)[0])
			{
				if (argv_append(cmd, *cur))
					return (1);
				*cur = ms_strdup("");
				if (!*cur)
					return (1);
			}
			while (val[i] && is_ifs(val[i]))
				i++;
		}
		else
			*cur = append_char(*cur, val[i++]);
		if (!*cur)
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
	char	*cur;
	char	*val;
	int		i;
	int		quoted;

	cur = ms_strdup("");
	if (!cur)
		return (1);
	quoted = 0;
	i = 0;
	while (s && s[i])
	{
		if (s[i] == '\'')
		{
			quoted = 1;
			cur = handle_single(s, &i, cur);
		}
		else if (s[i] == '"')
		{
			quoted = 1;
			cur = handle_double(s, &i, cur, sh);
		}
		else if (s[i] == '$' && s[i + 1])
		{
			val = expand_dollar(s, &i, sh);
			if (!val || append_split(cmd, &cur, val))
				return (free(val), free(cur), 1);
			free(val);
		}
		else
			cur = append_char(cur, s[i++]);
		if (!cur)
			return (1);
	}
	if (cur[0] || quoted)
		return (argv_append(cmd, cur));
	return (free(cur), 0);
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
