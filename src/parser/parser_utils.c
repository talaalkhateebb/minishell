/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   parser_utils.c                                       :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_redir(t_token_type type)
{
	return (type == T_REDIR_IN || type == T_REDIR_OUT
		|| type == T_APPEND || type == T_HEREDOC);
}

static int	has_ifs(const char *s)
{
	while (s && *s)
	{
		if (*s == ' ' || *s == '\t' || *s == '\n')
			return (1);
		s++;
	}
	return (0);
}

static int	has_unquoted_dollar(const char *s)
{
	char	quote;
	int		i;

	quote = 0;
	i = 0;
	while (s && s[i])
	{
		if (!quote && (s[i] == '\'' || s[i] == '"'))
			quote = s[i];
		else if (quote && s[i] == quote)
			quote = 0;
		else if (!quote && s[i] == '$')
			return (1);
		i++;
	}
	return (0);
}

int	is_reserved_word(const char *s)
{
	return (ms_strcmp(s, "in") == 0 || ms_strcmp(s, "do") == 0
		|| ms_strcmp(s, "then") == 0 || ms_strcmp(s, "else") == 0
		|| ms_strcmp(s, "elif") == 0 || ms_strcmp(s, "fi") == 0
		|| ms_strcmp(s, "esac") == 0);
}

/*
** bash: an unquoted `$VAR` in a redirect that expands to nothing or to more
** than one field is an ambiguous redirect. Message keeps the raw token.
*/
char	*expand_redir_target(t_token *target, t_shell *sh)
{
	char	*expanded;

	expanded = expand_word(target->value, sh);
	if (!expanded)
		return (NULL);
	if (has_unquoted_dollar(target->value)
		&& (!expanded[0] || has_ifs(expanded)))
	{
		put_err(target->value, "ambiguous redirect");
		sh->last_status = 1;
		free(expanded);
		return (NULL);
	}
	return (expanded);
}
