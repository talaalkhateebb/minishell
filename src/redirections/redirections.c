/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   redirections.c                                       :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/* Redirections — Person B. open() + dup2() + close() per t_redir entry.
** Applied left to right, so `> a > b` leaves stdout on b (and still
** creates a), exactly like bash. */

static int	open_target(t_redir *r)
{
	if (r->type == T_REDIR_IN)
		return (open(r->target, O_RDONLY));
	if (r->type == T_REDIR_OUT)
		return (open(r->target, O_WRONLY | O_CREAT | O_TRUNC, 0644));
	return (open(r->target, O_WRONLY | O_CREAT | O_APPEND, 0644));
}

static int	dup_heredoc(t_redir *r)
{
	if (r->heredoc_fd < 0)
		return (-1);
	if (dup2(r->heredoc_fd, STDIN_FILENO) == -1)
		return (-1);
	return (0);
}

static int	apply_one_redir(t_redir *r)
{
	int	fd;
	int	target_fd;

	if (r->type == T_HEREDOC)
		return (dup_heredoc(r));
	fd = open_target(r);
	if (fd == -1)
	{
		put_err(r->target, strerror(errno));
		return (-1);
	}
	if (r->type == T_REDIR_IN)
		target_fd = STDIN_FILENO;
	else
		target_fd = STDOUT_FILENO;
	if (dup2(fd, target_fd) == -1)
		return (close(fd), -1);
	close(fd);
	return (0);
}

int	apply_redirs(t_cmd *cmd)
{
	t_redir	*r;

	r = cmd->redirs;
	while (r)
	{
		if (apply_one_redir(r) == -1)
			return (-1);
		r = r->next;
	}
	return (0);
}
