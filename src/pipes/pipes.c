/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   pipes.c                                              :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Pipes — Person B (backend). N-1 pipes, N forks; every child must close
** every pipe fd or the pipeline hangs waiting on an fd nobody closed.
** Pipeline status = last command's status.
**
** A single external command (no pipe) also goes through here with n == 1:
** no pipe() calls happen, child_wire_pipes() is a no-op, and it just
** forks + execs. Single-command builtins never reach this file —
** executor.c runs those directly in the parent.
*/

static void	child_wire_pipes(int (*pipes)[2], int n, int idx)
{
	if (idx > 0)
		dup2(pipes[idx - 1][0], STDIN_FILENO);
	if (idx < n - 1)
		dup2(pipes[idx][1], STDOUT_FILENO);
	close_pipes(pipes, n - 1);
}

/*
** apply_redirs() consumes THIS command's heredoc (dup2 onto stdin, then
** close, then mark it -1). The read ends belonging to the OTHER commands in
** the pipeline are still open in this child, inherited from the fork, and
** execve() would carry them into the program — so they go first. Under
** valgrind --track-fds they show up as leaked descriptors; without it they
** are simply fds no one will ever read.
*/
static void	run_child(t_cmd *cmd, t_shell *sh)
{
	char	*exec_path;
	char	**env;

	setup_signals_child();
	if (apply_redirs(cmd) == -1)
		child_exit(sh, 1);
	close_heredocs(sh->cmds);
	if (!cmd->argv[0])
		child_exit(sh, 0);
	if (is_builtin(cmd->argv[0]))
		child_exit(sh, run_builtin(cmd, sh));
	exec_path = find_executable(cmd->argv[0], sh);
	if (!exec_path)
		child_exec_fail(cmd, sh);
	env = env_to_array(sh);
	execve(exec_path, cmd->argv, env);
	if (errno == ENOEXEC)
		exec_as_shell_script(exec_path, cmd->argv, env);
	put_err(cmd->argv[0], strerror(errno));
	free(exec_path);
	child_exit(sh, 126);
}

static int	open_all_pipes(int (*pipes)[2], int n)
{
	int	i;

	i = 0;
	while (i < n - 1)
	{
		if (pipe(pipes[i]) == -1)
		{
			close_pipes(pipes, i);
			return (-1);
		}
		i++;
	}
	return (0);
}

static int	fork_all(t_cmd *cmds, t_shell *sh, int (*pipes)[2], pid_t *pids)
{
	int		i;
	int		n;
	t_cmd	*cur;

	n = count_cmds(cmds);
	i = 0;
	while (i < n)
		pids[i++] = -1;
	cur = cmds;
	i = 0;
	while (cur)
	{
		pids[i] = fork();
		if (pids[i] == -1)
			return (-1);
		if (pids[i] == 0)
		{
			child_wire_pipes(pipes, n, i);
			run_child(cur, sh);
		}
		cur = cur->next;
		i++;
	}
	return (0);
}

int	run_pipeline(t_cmd *cmds, t_shell *sh)
{
	int		(*pipes)[2];
	pid_t	*pids;
	int		n;
	int		status;
	int		slots;

	n = count_cmds(cmds);
	slots = n - 1;
	if (slots < 1)
		slots = 1;
	pipes = malloc(sizeof(int [2]) * slots);
	pids = malloc(sizeof(pid_t) * n);
	if (!pipes || !pids || open_all_pipes(pipes, n) == -1)
		return (free(pipes), free(pids), 1);
	setup_signals_exec();
	if (fork_all(cmds, sh, pipes, pids) == -1)
		put_err(NULL, "fork failed");
	close_pipes(pipes, n - 1);
	close_heredocs(cmds);
	free(pipes);
	status = wait_children(pids, n);
	free(pids);
	setup_signals_interactive();
	return (status);
}
