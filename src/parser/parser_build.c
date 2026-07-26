/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   parser_build.c                                       :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_cmd	*cmd_new(void)
{
	t_cmd	*cmd;

	cmd = malloc(sizeof(t_cmd));
	if (!cmd)
		return (NULL);
	cmd->argv = malloc(sizeof(char *));
	if (!cmd->argv)
		return (free(cmd), NULL);
	cmd->argv[0] = NULL;
	cmd->redirs = NULL;
	cmd->next = NULL;
	return (cmd);
}

/* Takes ownership of `value`. */
int	argv_append(t_cmd *cmd, char *value)
{
	char	**bigger;
	int		n;
	int		i;

	if (!value)
		return (1);
	n = 0;
	while (cmd->argv[n])
		n++;
	bigger = malloc(sizeof(char *) * (n + 2));
	if (!bigger)
		return (free(value), 1);
	i = 0;
	while (i < n)
	{
		bigger[i] = cmd->argv[i];
		i++;
	}
	bigger[n] = value;
	bigger[n + 1] = NULL;
	free(cmd->argv);
	cmd->argv = bigger;
	return (0);
}

/*
** Expands one word token and attaches the resulting fields. Unquoted
** expansions are split on IFS; an unquoted word that expands to nothing
** disappears (`$EMPTY echo hi` → `echo hi`). Quotes keep an empty
** argument alive, so `echo ""` still prints a blank line.
*/
int	word_append(t_cmd *cmd, t_token *tok, t_shell *sh)
{
	return (expand_to_argv(cmd, tok->value, sh));
}

/* Takes ownership of `target`. */
int	redir_append(t_cmd *cmd, t_token_type type, char *target, int expand)
{
	t_redir	*redir;
	t_redir	*cur;

	if (!target)
		return (1);
	redir = malloc(sizeof(t_redir));
	if (!redir)
		return (free(target), 1);
	redir->type = type;
	redir->target = target;
	redir->expand_heredoc = expand;
	redir->heredoc_fd = -1;
	redir->next = NULL;
	if (!cmd->redirs)
		return (cmd->redirs = redir, 0);
	cur = cmd->redirs;
	while (cur->next)
		cur = cur->next;
	cur->next = redir;
	return (0);
}
