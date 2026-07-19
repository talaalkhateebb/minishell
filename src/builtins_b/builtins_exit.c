/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_exit.c                                      :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_numeric_arg(const char *s)
{
	int	i;

	i = 0;
	if (s[i] == '+' || s[i] == '-')
		i++;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static long	ms_atol(const char *s)
{
	long	res;
	long	sign;
	int		i;

	res = 0;
	sign = 1;
	i = 0;
	if (s[i] == '+' || s[i] == '-')
	{
		if (s[i] == '-')
			sign = -1;
		i++;
	}
	while (s[i] >= '0' && s[i] <= '9')
	{
		res = res * 10 + (s[i] - '0');
		i++;
	}
	return (res * sign);
}

/*
** `exit` prints "exit" and leaves with the last status. A non-numeric
** argument is a fatal error (status 2). Too many arguments is NOT fatal:
** bash complains and stays in the shell, so this returns 1 instead of
** exiting.
*/
int	builtin_exit(char **argv, t_shell *sh)
{
	long	code;

	if (is_interactive())
		put_str(1, "exit\n");
	if (argv[1] && !is_numeric_arg(argv[1]))
	{
		put_err("exit", "numeric argument required");
		env_free(sh);
		exit(2);
	}
	if (argv[1] && argv[2])
		return (put_err("exit", "too many arguments"), 1);
	if (argv[1])
		code = ms_atol(argv[1]) % 256;
	else
		code = sh->last_status;
	if (code < 0)
		code += 256;
	env_free(sh);
	exit((int)code);
}
