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
** Built-ins owned by Person B: cd, pwd, exit, `.` (source stub).
*/

/*
** bash's `.` / source. Full file sourcing is out of scope for the
** mandatory part; the no-argument case must match bash's message.
*/
int	builtin_dot(char **argv)
{
	if (!argv[1])
	{
		put_err(".", "filename argument required");
		put_str(2, ".: usage: . filename [arguments]\n");
		return (2);
	}
	put_err(argv[1], "No such file or directory");
	return (1);
}

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
	char	*old;
	char	*new;

	if (argv[1] && argv[2])
		return (put_err("cd", "too many arguments"), 1);
	if (argv[1] && !argv[1][0])
		return (0);
	target = resolve_cd_target(argv, sh);
	if (!target)
		return (1);
	old = cd_current_pwd(sh);
	new = cd_logical_path(sh, target);
	if (!old || !new)
		return (free(old), free(new), 1);
	if (cd_move(target, new) == -1)
		return (free(old), free(new), cd_error(target), 1);
	env_set(sh, "OLDPWD", old);
	env_set(sh, "PWD", new);
	return (free(old), free(new), 0);
}

int	builtin_pwd(t_shell *sh)
{
	char	*pwd;

	pwd = cd_current_pwd(sh);
	if (!pwd || !pwd[0])
		return (free(pwd), put_err("pwd", strerror(errno)), 1);
	put_str(1, pwd);
	put_str(1, "\n");
	return (free(pwd), 0);
}
