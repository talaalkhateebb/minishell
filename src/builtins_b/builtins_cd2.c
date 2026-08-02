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
