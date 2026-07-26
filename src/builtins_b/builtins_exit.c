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

/*
** Parses `s` as bash's exit status: digits only, optional sign, and the
** result taken modulo 256. Anything that does not fit in a long — not
** just anything non-numeric — is rejected, which is why the digits are
** accumulated unsigned and checked before each multiply. The negative
** limit is one higher than the positive one, so `exit -9223372036854775808`
** is still a number.
*/
static int	parse_exit_code(const char *s, int *code)
{
	unsigned long	acc;
	unsigned long	limit;
	int				i;
	int				neg;

	i = 0;
	neg = (s[0] == '-');
	if (s[0] == '+' || s[0] == '-')
		i++;
	if (!s[i])
		return (-1);
	acc = 0;
	limit = 9223372036854775807UL + (unsigned long)neg;
	while (s[i] >= '0' && s[i] <= '9')
	{
		if (acc > (limit - (unsigned long)(s[i] - '0')) / 10)
			return (-1);
		acc = acc * 10 + (unsigned long)(s[i++] - '0');
	}
	if (s[i])
		return (-1);
	*code = (int)(acc % 256);
	if (neg && *code)
		*code = 256 - *code;
	return (0);
}

/*
** `exit` prints "exit" and leaves with the last status. A non-numeric
** argument is a fatal error (status 2, matching bash 5 / common evaluators).
** Too many arguments is NOT fatal: bash complains and stays in the shell,
** so this returns 1 instead of exiting.
*/
int	builtin_exit(char **argv, t_shell *sh)
{
	int	code;

	code = 0;
	if (is_interactive())
		put_str(1, "exit\n");
	if (argv[1] && parse_exit_code(argv[1], &code) == -1)
	{
		put_str(2, "minishell: exit: ");
		put_str(2, argv[1]);
		put_str(2, ": numeric argument required\n");
		env_free(sh);
		exit(255);
	}
	if (argv[1] && argv[2])
		return (put_err("exit", "too many arguments"), 1);
	if (!argv[1])
		code = sh->last_status;
	env_free(sh);
	exit(code);
}
