/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   heredoc.c                                            :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Heredocs — Person B (backend).
**
** All heredocs in a pipeline are read up front, in the parent shell,
** before anything forks. Each gets its own pipe: the body is written into
** fds[1], which is then closed, and fds[0] is stashed on the t_redir so
** redirections.c can dup2() it onto stdin later.
**
** $VAR expands in the body only when the delimiter was UNQUOTED — the
** parser already worked that out and left it in redir->expand_heredoc.
*/

static void	warn_heredoc_eof(t_redir *redir)
{
	put_str(2, "minishell: warning: here-document at line 1 "
		"delimited by end-of-file (wanted `");
	put_str(2, redir->target);
	put_str(2, "')\n");
}

static int	write_heredoc_line(int fd, char *line, t_redir *redir,
	t_shell *sh)
{
	char	*expanded;

	expanded = line;
	if (redir->expand_heredoc)
		expanded = expand_heredoc_line(line, sh);
	put_str(fd, expanded);
	put_str(fd, "\n");
	if (expanded != line)
		free(expanded);
	return (0);
}

/* Returns 0 on the delimiter, 1 if Ctrl-C cancelled it, 2 on EOF. */
static int	read_heredoc_body(int fd, t_redir *redir, t_shell *sh)
{
	char	*line;

	while (1)
	{
		line = ms_readline("> ");
		if (!line)
		{
			if (g_signal == SIGINT)
				return (1);
			warn_heredoc_eof(redir);
			return (2);
		}
		if (ms_strcmp(line, redir->target) == 0)
			return (free(line), 0);
		write_heredoc_line(fd, line, redir, sh);
		free(line);
	}
}

/*
** The heredoc SIGINT handler closes fd 0 to break readline() out of its
** read. That has to be undone here, or the shell loses its stdin for good.
*/
static int	fill_heredoc(t_redir *redir, t_shell *sh)
{
	int	fds[2];
	int	saved_stdin;
	int	result;

	if (pipe(fds) == -1)
		return (-1);
	saved_stdin = dup(STDIN_FILENO);
	if (saved_stdin == -1)
		return (close(fds[0]), close(fds[1]), -1);
	g_signal = 0;
	setup_signals_heredoc();
	result = read_heredoc_body(fds[1], redir, sh);
	setup_signals_interactive();
	dup2(saved_stdin, STDIN_FILENO);
	close(saved_stdin);
	close(fds[1]);
	if (result == 1)
		return (close(fds[0]), -1);
	redir->heredoc_fd = fds[0];
	return (0);
}

int	process_heredocs(t_cmd *cmds, t_shell *sh)
{
	t_cmd	*cur;
	t_redir	*r;

	cur = cmds;
	while (cur)
	{
		r = cur->redirs;
		while (r)
		{
			if (r->type == T_HEREDOC && fill_heredoc(r, sh) == -1)
			{
				sh->last_status = 130;
				return (-1);
			}
			r = r->next;
		}
		cur = cur->next;
	}
	return (0);
}
