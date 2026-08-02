/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   executor_child.c                                     :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Leaving a forked child.
**
** execve() replaces the image, so a child that execs takes nothing with it.
** A child that does NOT exec — a built-in inside a pipeline, a failed
** redirection, a command that could not be found, an empty argv — reaches
** exit() still holding every allocation the parent had made: the
** environment, the command list, the line readline handed back and the
** history it was added to. exit() abandons all of it, and a leak checker
** that follows the forks (valgrind --trace-children=yes) then reports one
** full copy of the shell per child.
**
** So the child gives it all back first. The parent's own copies are
** untouched — this runs after fork(), on the child's private pages.
*/
void	child_exit(t_shell *sh, int code)
{
	free_cmds(sh->cmds);
	sh->cmds = NULL;
	free(sh->line);
	sh->line = NULL;
	env_free(sh);
	rl_clear_history();
	exit(code);
}

/*
** Nothing runnable came back from the PATH search. Kept out of run_child()
** so that both the message and the exit still go through child_exit().
**
** bash reports a bare name as "No such file or directory" rather than
** "command not found" when there is no PATH at all to search.
*/
void	child_exec_fail(t_cmd *cmd, t_shell *sh)
{
	if (!has_slash(cmd->argv[0]) && path_is_unset(sh))
	{
		put_err(cmd->argv[0], "No such file or directory");
		child_exit(sh, 127);
	}
	child_exit(sh, report_exec_error(cmd->argv[0]));
}
