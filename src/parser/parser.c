/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   parser.c                                             :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Parser — Person A (frontend).
**
** parse_line is the single entry point the rest of the shell uses:
**   line → tokenize() → parse_tokens() (which expands as it goes) → t_cmd
**
** Returns NULL on a syntax error or an empty line, having already set
** sh->last_status (2 for a syntax error, as bash does).
*/

/*
** Stash the token and status only — the message is printed after any
** pending here-documents are read, matching bash's order for
** `cat << EOF >` (body first, then the syntax error).
*/
void	syntax_error(const char *near, t_shell *sh)
{
	sh->last_status = 2;
	free(sh->syntax_token);
	sh->syntax_token = ms_strdup(near);
}

t_cmd	*parse_line(const char *line, t_shell *sh)
{
	t_token	*tokens;
	t_cmd	*cmds;
	int		err;

	tokens = tokenize(line, &err);
	if (err)
	{
		put_str(2, "minishell: unexpected EOF while looking for"
			" matching quote\n");
		sh->last_status = 2;
		return (NULL);
	}
	if (!tokens)
		return (NULL);
	cmds = parse_tokens(tokens, sh);
	free_tokens(tokens);
	return (cmds);
}

static void	free_argv(char **argv)
{
	free_array(argv);
}

static void	free_redirs(t_redir *redirs)
{
	t_redir	*next;

	while (redirs)
	{
		next = redirs->next;
		if (redirs->type == T_HEREDOC && redirs->heredoc_fd >= 0)
			close(redirs->heredoc_fd);
		free(redirs->target);
		free(redirs);
		redirs = next;
	}
}

void	free_cmds(t_cmd *cmds)
{
	t_cmd	*next;

	while (cmds)
	{
		next = cmds->next;
		free_argv(cmds->argv);
		free_redirs(cmds->redirs);
		free(cmds);
		cmds = next;
	}
}
