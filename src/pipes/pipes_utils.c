/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   pipes_utils.c                                        :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	count_cmds(t_cmd *cmds)
{
	int	n;

	n = 0;
	while (cmds)
	{
		n++;
		cmds = cmds->next;
	}
	return (n);
}

/*
** A pipeline that could not even be set up: release the two tables and
** hand back the status. Named so that run_pipeline() can bail out with a
** single value rather than returning the cleanup and the status together.
*/
int	pipeline_fail(int (*pipes)[2], pid_t *pids)
{
	free(pipes);
	free(pids);
	return (1);
}

void	close_pipes(int (*pipes)[2], int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
		i++;
	}
}

/* Heredoc read-ends live in the parent; children inherit copies. Once the
** forks are done the parent's copies must go, or `cat << EOF` never sees
** EOF on stdin. */
void	close_heredocs(t_cmd *cmds)
{
	t_redir	*r;

	while (cmds)
	{
		r = cmds->redirs;
		while (r)
		{
			if (r->type == T_HEREDOC && r->heredoc_fd >= 0)
			{
				close(r->heredoc_fd);
				r->heredoc_fd = -1;
			}
			r = r->next;
		}
		cmds = cmds->next;
	}
}

/*
** Only the LAST command's status becomes the pipeline's status, but every
** child must still be reaped. A child killed by a signal reports
** 128 + signo, and Ctrl-C / Ctrl-\ print their newline or message once.
*/
int	wait_children(pid_t *pids, int n)
{
	int	status;
	int	last;
	int	i;

	last = 0;
	i = 0;
	while (i < n)
	{
		if (pids[i] == -1 || waitpid(pids[i], &status, 0) == -1)
			status = 0;
		if (i == n - 1)
		{
			if (WIFEXITED(status))
				last = WEXITSTATUS(status);
			else if (WIFSIGNALED(status))
				last = report_signal(status);
		}
		i++;
	}
	return (last);
}
