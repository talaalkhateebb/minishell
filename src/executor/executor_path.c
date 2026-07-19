/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   executor_path.c                                      :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** PATH resolution.
**
** This walks sh->envp rather than getenv(), because export/unset manage
** the shell's own environment array and never touch the process
** environment — so `export PATH=...` has to be visible here.
*/

static char	*join_path(const char *dir, const char *cmd)
{
	char	*path;
	size_t	i;
	size_t	j;

	path = malloc(ms_strlen(dir) + ms_strlen(cmd) + 2);
	if (!path)
		return (NULL);
	i = 0;
	while (dir[i])
	{
		path[i] = dir[i];
		i++;
	}
	path[i++] = '/';
	j = 0;
	while (cmd[j])
		path[i++] = cmd[j++];
	path[i] = '\0';
	return (path);
}

static int	has_slash(const char *s)
{
	while (*s)
	{
		if (*s == '/')
			return (1);
		s++;
	}
	return (0);
}

static char	*next_dir(char **cursor)
{
	char	*start;
	char	*dir;
	int		len;

	start = *cursor;
	len = 0;
	while (start[len] && start[len] != ':')
		len++;
	dir = ms_substr(start, 0, len);
	if (!dir)
		return (NULL);
	*cursor = start + len;
	if (**cursor == ':')
		(*cursor)++;
	return (dir);
}

static char	*search_in_path(char *cmd, t_shell *sh)
{
	char	*cursor;
	char	*dir;
	char	*candidate;

	cursor = env_get(sh, "PATH");
	if (!cursor || !*cursor)
		return (NULL);
	while (*cursor)
	{
		dir = next_dir(&cursor);
		if (!dir)
			return (NULL);
		candidate = join_path(dir, cmd);
		free(dir);
		if (candidate && access(candidate, X_OK) == 0)
			return (candidate);
		free(candidate);
	}
	return (NULL);
}

char	*find_executable(char *cmd, t_shell *sh)
{
	if (!cmd || !*cmd)
		return (NULL);
	if (has_slash(cmd))
	{
		if (access(cmd, X_OK) == 0)
			return (ms_strdup(cmd));
		return (NULL);
	}
	return (search_in_path(cmd, sh));
}
