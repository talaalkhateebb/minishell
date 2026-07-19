/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_b.c                                         :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Built-ins owned by Person B: cd, pwd, exit.
*/

/* `cd` with no argument uses HOME; `cd -` returns to OLDPWD and echoes
** the destination, both of which come from the env module. */
static char	*resolve_cd_target(char **argv, t_shell *sh)
{
	char	*target;

	if (!argv[1] || ms_strcmp(argv[1], "~") == 0)
	{
		target = env_get(sh, "HOME");
		if (!target)
			put_err("cd", "HOME not set");
		return (target);
	}
	if (ms_strcmp(argv[1], "-") == 0)
	{
		target = env_get(sh, "OLDPWD");
		if (!target)
			return (put_err("cd", "OLDPWD not set"), NULL);
		put_str(1, target);
		put_str(1, "\n");
		return (target);
	}
	return (argv[1]);
}

/* bash reports `cd: <path>: <reason>`, keeping the builtin's name. */
static void	cd_error(const char *target)
{
	put_str(2, "minishell: cd: ");
	put_str(2, target);
	put_str(2, ": ");
	put_str(2, strerror(errno));
	put_str(2, "\n");
}

int	builtin_cd(char **argv, t_shell *sh)
{
	char	*target;
	char	old[4096];
	char	new[4096];

	if (argv[1] && argv[2])
		return (put_err("cd", "too many arguments"), 1);
	target = resolve_cd_target(argv, sh);
	if (!target)
		return (1);
	if (!getcwd(old, sizeof(old)))
		old[0] = '\0';
	if (chdir(target) == -1)
		return (cd_error(target), 1);
	if (old[0])
		env_set(sh, "OLDPWD", old);
	if (getcwd(new, sizeof(new)))
		env_set(sh, "PWD", new);
	return (0);
}

int	builtin_pwd(void)
{
	char	buf[4096];

	if (!getcwd(buf, sizeof(buf)))
		return (put_err("pwd", strerror(errno)), 1);
	put_str(1, buf);
	put_str(1, "\n");
	return (0);
}
