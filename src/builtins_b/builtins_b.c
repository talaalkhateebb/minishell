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
** the destination, both of which come from the env module. `--` ends the
** options, so it means HOME too, and a bare `~` only reaches us unexpanded
** when HOME is unset — the same lookup reports that. */
static char	*resolve_cd_target(char **argv, t_shell *sh)
{
	char	*target;

	if (!argv[1] || ms_strcmp(argv[1], "~") == 0
		|| ms_strcmp(argv[1], "--") == 0)
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
		{
			put_err("cd", "OLDPWD not set");
			return (NULL);
		}
		put_str(1, target);
		put_str(1, "\n");
		return (target);
	}
	return (argv[1]);
}

/*
** The argument checks and the move itself live in builtins_cd2.c: spelling
** each branch out as its own statements — rather than returning a cleanup
** call and a status together — costs more lines than this function has.
*/
int	builtin_cd(char **argv, t_shell *sh)
{
	char	*target;
	char	*old;
	char	*new;

	if (cd_reject_args(argv))
		return (1);
	target = resolve_cd_target(argv, sh);
	if (!target)
		return (1);
	old = cd_current_pwd(sh);
	new = cd_target_path(sh, target, cd_report_lost_cwd(sh));
	if (!old || !new)
	{
		free(old);
		free(new);
		return (1);
	}
	return (cd_apply(sh, target, old, new));
}

int	builtin_pwd(t_shell *sh)
{
	char	*pwd;

	pwd = cd_current_pwd(sh);
	if (!pwd || !pwd[0])
	{
		free(pwd);
		return (err_ret("pwd", strerror(errno), 1));
	}
	put_str(1, pwd);
	put_str(1, "\n");
	free(pwd);
	return (0);
}
