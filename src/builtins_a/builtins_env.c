/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_env.c                                       :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** `env CMD [args]` — the argument form of env. It runs CMD with the shell's
** environment, so it has to fork: env is a built-in here, and execve() in
** the parent would replace the shell itself.
*/

/*
** env speaks under its own name and reports plain execve errors: a bare
** name it cannot find is "No such file or directory", not bash's "command
** not found", and a directory comes back as "Permission denied" (126)
** because that is the errno execve() gives it.
**
** The single quotes are GNU coreutils' doing, not ours — every coreutils
** tool quotes the operand it is complaining about ("ls: cannot access
** 'foo'"), so the Ubuntu bash this is measured against prints
** `env: 'test/': ...`. The BSD env on macOS leaves the quotes out.
*/
static void	env_exec_error(const char *name, int code)
{
	put_str(2, "env: '");
	put_str(2, name);
	put_str(2, "': ");
	put_str(2, strerror(code));
	put_str(2, "\n");
	if (code == ENOENT)
		exit(127);
	exit(126);
}

static void	env_exec_child(char **av, t_shell *sh)
{
	char	*path;
	char	**env;

	setup_signals_child();
	if (has_slash(av[0]))
		path = ms_strdup(av[0]);
	else
		path = find_executable(av[0], sh);
	if (!path)
		env_exec_error(av[0], ENOENT);
	env = env_to_array(sh);
	execve(path, av, env);
	if (errno == ENOEXEC)
		exec_as_shell_script(path, av, env);
	env_exec_error(av[0], errno);
}

int	env_run_command(char **av, t_shell *sh)
{
	pid_t	pid;
	int		status;

	setup_signals_exec();
	pid = fork();
	if (pid == -1)
	{
		setup_signals_interactive();
		return (put_err("env", "fork failed"), 1);
	}
	if (pid == 0)
		env_exec_child(av, sh);
	status = wait_children(&pid, 1);
	setup_signals_interactive();
	return (status);
}
