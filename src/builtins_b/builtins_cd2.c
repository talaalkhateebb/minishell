/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_cd2.c                                       :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Standing in a directory that has been deleted underneath us.
**
** bash warns once, from cd, and then carries on: the chdir still happens,
** so the status stays 0 and only the message tells you the shell no longer
** knows where it is. The wording is bash's own, down to the embedded
** "getcwd:" — it comes from get_working_directory(), not from strerror().
**
** Only once, though: having warned, bash writes down where it thinks it is
** and trusts that from then on, so a second `cd ..` inside the deleted tree
** is silent and back to ordinary logical resolution. cwd_lost remembers
** that, and clears as soon as getcwd() answers again.
*/
int	cd_report_lost_cwd(t_shell *sh)
{
	char	buf[4096];

	if (getcwd(buf, sizeof(buf)))
	{
		sh->cwd_lost = 0;
		return (0);
	}
	if (sh->cwd_lost)
		return (0);
	sh->cwd_lost = 1;
	put_err(NULL, "error retrieving current directory: getcwd: "
		"cannot access parent directories: No such file or directory");
	return (1);
}

/*
** Where cd will say it landed. Normally that is the canonical logical path,
** but a shell that has lost its cwd cannot canonicalize anything — there is
** no filesystem answer to check `..` against — so bash just glues the target
** onto the stale $PWD and leaves it uncollapsed. That is why bash reports
** `a/b/..` rather than `a` after the directory is deleted underneath it.
*/
char	*cd_target_path(t_shell *sh, const char *target, int lost)
{
	char	*res;

	if (!lost || target[0] == '/')
		return (cd_logical_path(sh, target));
	res = cd_current_pwd(sh);
	res = append_char(res, '/');
	return (append_str(res, target));
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

/*
** The two argument shapes bash refuses before it resolves anything: more
** than one operand, and an empty one. Non-zero means it has complained
** already and cd should stop.
*/
int	cd_reject_args(char **argv)
{
	if (argv[1] && argv[2])
		return (err_ret("cd", "too many arguments", 1));
	if (argv[1] && !argv[1][0])
		return (err_ret("cd", "null directory", 1));
	return (0);
}

/*
** Performs the move and records it. Takes ownership of `old` and `new`,
** which are freed on every path — that ownership is what lets builtin_cd()
** hand back a single value instead of a cleanup-and-status pair.
*/
int	cd_apply(t_shell *sh, const char *target, char *old, char *dest)
{
	int	status;

	status = 0;
	if (cd_move(target, dest) == -1)
	{
		cd_error(target);
		status = 1;
	}
	else
	{
		env_set(sh, "OLDPWD", old);
		env_set(sh, "PWD", dest);
	}
	free(old);
	free(dest);
	return (status);
}
