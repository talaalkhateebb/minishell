/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   executor.c                                           :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Executor — Person B (backend).
**
** A single built-in with no pipe runs in the PARENT, so that `cd`, `export`
** and `unset` can actually change this shell's state. Everything else goes
** through run_pipeline() and runs in forked children.
*/

int	is_builtin(const char *cmd)
{
	if (!cmd)
		return (0);
	if (ms_strcmp(cmd, "echo") == 0 || ms_strcmp(cmd, "cd") == 0
		|| ms_strcmp(cmd, "pwd") == 0 || ms_strcmp(cmd, "export") == 0
		|| ms_strcmp(cmd, "unset") == 0 || ms_strcmp(cmd, "env") == 0
		|| ms_strcmp(cmd, "exit") == 0 || ms_strcmp(cmd, ".") == 0)
		return (1);
	return (0);
}

int	run_builtin(t_cmd *cmd, t_shell *sh)
{
	char	*name;

	name = cmd->argv[0];
	if (ms_strcmp(name, "echo") == 0)
		return (builtin_echo(cmd->argv));
	if (ms_strcmp(name, "cd") == 0)
		return (builtin_cd(cmd->argv, sh));
	if (ms_strcmp(name, "pwd") == 0)
		return (builtin_pwd());
	if (ms_strcmp(name, "export") == 0)
		return (builtin_export(cmd->argv, sh));
	if (ms_strcmp(name, "unset") == 0)
		return (builtin_unset(cmd->argv, sh));
	if (ms_strcmp(name, "env") == 0)
		return (builtin_env(sh));
	if (ms_strcmp(name, ".") == 0)
		return (builtin_dot(cmd->argv));
	return (builtin_exit(cmd->argv, sh));
}

/*
** Redirections are applied around the built-in and then undone, so a
** `export FOO=bar > file` doesn't leave the shell's stdout redirected.
*/
static int	execute_builtin_parent(t_cmd *cmd, t_shell *sh)
{
	int	saved_in;
	int	saved_out;
	int	status;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	if (apply_redirs(cmd) == -1)
		status = 1;
	else
		status = run_builtin(cmd, sh);
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (status);
}

/*
** A command with redirections but no words (`> out`) still has to create
** and truncate the file — it just runs nothing afterwards.
*/
static int	redirs_only(t_cmd *cmd)
{
	int	saved_in;
	int	saved_out;
	int	status;

	saved_in = dup(STDIN_FILENO);
	saved_out = dup(STDOUT_FILENO);
	status = 0;
	if (apply_redirs(cmd) == -1)
		status = 1;
	dup2(saved_in, STDIN_FILENO);
	dup2(saved_out, STDOUT_FILENO);
	close(saved_in);
	close(saved_out);
	return (status);
}

int	execute(t_cmd *cmds, t_shell *sh)
{
	if (!cmds)
		return (sh->last_status);
	if (process_heredocs(cmds, sh) == -1)
		return (close_heredocs(cmds), 130);
	if (!cmds->next && !cmds->argv[0])
		return (redirs_only(cmds));
	if (!cmds->next && is_builtin(cmds->argv[0]))
		return (execute_builtin_parent(cmds, sh));
	return (run_pipeline(cmds, sh));
}
