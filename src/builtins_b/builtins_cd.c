/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_cd.c                                        :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Logical path handling for cd/pwd.
**
** bash keeps $PWD as the path you typed, not the one getcwd() reports: on a
** system where /tmp is a symlink to /private/tmp, `cd /tmp` then `pwd`
** prints /tmp, and a following `cd ..` lands in / rather than /private.
** That only works if `.` and `..` are resolved textually, without asking
** the filesystem — which is what canonicalize() does.
*/

/* Appends one component, or drops the previous one when it is "..". */
static char	*add_component(char *res, const char *comp, size_t len)
{
	size_t	i;

	if (len == 0 || (len == 1 && comp[0] == '.'))
		return (res);
	if (len == 2 && comp[0] == '.' && comp[1] == '.')
	{
		i = ms_strlen(res);
		while (i > 0 && res[i - 1] != '/')
			i--;
		if (i > 0)
			i--;
		res[i] = '\0';
		return (res);
	}
	res = append_char(res, '/');
	i = 0;
	while (res && i < len)
		res = append_char(res, comp[i++]);
	return (res);
}

static char	*canonicalize(const char *path)
{
	char	*res;
	int		i;
	int		start;

	res = ms_strdup("");
	i = 0;
	while (res && path[i])
	{
		while (path[i] == '/')
			i++;
		start = i;
		while (path[i] && path[i] != '/')
			i++;
		if (i > start)
			res = add_component(res, path + start, i - start);
	}
	if (res && !res[0])
	{
		free(res);
		return (ms_strdup("/"));
	}
	return (res);
}

/*
** The shell's logical cwd. $PWD is only trusted when it still names the
** directory we are actually in, so a hand-edited `export PWD=...` cannot
** make pwd lie; otherwise getcwd() decides.
**
** When the directory we are standing in has been deleted, getcwd() fails
** and there is nothing left to verify $PWD against — but bash still
** resolves `cd ..` and `pwd` against it, so a stale $PWD is better than
** none. Dropping it here is what made `cd ..` resolve "" + "/.." and land
** in "/". This last fallback cannot make pwd lie: it is only reached once
** getcwd() has already refused to answer.
*/
char	*cd_current_pwd(t_shell *sh)
{
	struct stat	a;
	struct stat	b;
	char		*pwd;
	char		buf[4096];

	pwd = env_get(sh, "PWD");
	if (pwd && pwd[0] == '/' && stat(pwd, &a) == 0 && stat(".", &b) == 0
		&& a.st_dev == b.st_dev && a.st_ino == b.st_ino)
		return (ms_strdup(pwd));
	if (getcwd(buf, sizeof(buf)))
		return (ms_strdup(buf));
	if (pwd && pwd[0] == '/')
		return (ms_strdup(pwd));
	return (ms_strdup(""));
}

/* Where `target` lands, expressed logically against the current $PWD. */
char	*cd_logical_path(t_shell *sh, const char *target)
{
	char	*joined;
	char	*res;

	if (target[0] == '/')
		return (canonicalize(target));
	joined = cd_current_pwd(sh);
	joined = append_char(joined, '/');
	joined = append_str(joined, target);
	if (!joined)
		return (NULL);
	res = canonicalize(joined);
	free(joined);
	return (res);
}

/*
** Prefer the logical path, so symlinked components stay in $PWD; fall back
** to the literal target when the logical one does not resolve.
*/
int	cd_move(const char *target, const char *logical)
{
	if (chdir(logical) == 0)
		return (0);
	return (chdir(target));
}
