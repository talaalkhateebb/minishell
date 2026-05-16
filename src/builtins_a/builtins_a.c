#include "minishell.h"

/*
** Built-ins owned by Person A (env-touching): echo, env, export, unset.
** Day-0 stubs return 0. Real implementations on Days 10–11.
*/

int	builtin_echo(char **argv)
{
	(void)argv;
	write(1, "\n", 1);
	return (0);
}

int	builtin_env(t_shell *sh)
{
	int	i;

	if (!sh->envp)
		return (0);
	i = 0;
	while (sh->envp[i])
	{
		write(1, sh->envp[i], ms_strlen(sh->envp[i]));
		write(1, "\n", 1);
		i++;
	}
	return (0);
}

int	builtin_export(char **argv, t_shell *sh)
{
	(void)argv;
	(void)sh;
	return (0);
}

int	builtin_unset(char **argv, t_shell *sh)
{
	(void)argv;
	(void)sh;
	return (0);
}
