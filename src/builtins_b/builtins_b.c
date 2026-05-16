#include "minishell.h"

/*
** Built-ins owned by Person B: cd, pwd, exit. Day-0 stubs return 0.
*/

int	builtin_cd(char **argv, t_shell *sh)
{
	(void)argv;
	(void)sh;
	return (0);
}

int	builtin_pwd(void)
{
	char	buf[4096];

	if (!getcwd(buf, sizeof(buf)))
		return (1);
	write(1, buf, ms_strlen(buf));
	write(1, "\n", 1);
	return (0);
}

int	builtin_exit(char **argv, t_shell *sh)
{
	(void)argv;
	(void)sh;
	return (0);
}
