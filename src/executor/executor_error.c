/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   executor_error.c                                     :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** What a command name is, and what to say when it cannot be run.
** Shared by executor_path.c (which decides) and pipes.c (which reports).
*/

/*
** bash: if execve fails with ENOEXEC (e.g. empty +x file), retry as a
** /bin/sh script — that is why `./file_test` on an empty file exits 0.
*/
void	exec_as_shell_script(char *path, char **argv, char **env)
{
	char	**new_argv;
	int		n;
	int		i;

	n = 0;
	while (argv[n])
		n++;
	new_argv = malloc(sizeof(char *) * (n + 2));
	if (!new_argv)
		return ;
	new_argv[0] = "/bin/sh";
	new_argv[1] = path;
	i = 1;
	while (argv[i])
	{
		new_argv[i + 1] = argv[i];
		i++;
	}
	new_argv[i + 1] = NULL;
	execve("/bin/sh", new_argv, env);
	free(new_argv);
}

int	has_slash(const char *s)
{
	while (*s)
	{
		if (*s == '/')
			return (1);
		s++;
	}
	return (0);
}

/*
** A directory is "executable" as far as access(X_OK) goes — it means
** searchable — so it has to be rejected explicitly, or `$PWD` would be
** handed to execve() instead of being reported as a directory.
*/
int	is_exec_file(const char *path)
{
	struct stat	st;

	if (stat(path, &st) == -1 || S_ISDIR(st.st_mode))
		return (0);
	return (access(path, X_OK) == 0);
}

static void	strip_trailing_slashes(char *path)
{
	size_t	len;

	len = ms_strlen(path);
	while (len > 1 && path[len - 1] == '/')
	{
		path[len - 1] = '\0';
		len--;
	}
}

/*
** Says why find_executable() came back empty and gives bash's status for
** it: 127 when nothing was found, 126 when something is there but cannot
** be run. A bare name that misses in PATH is always "command not found",
** even when a file of that name sits in the current directory.
*/
int	report_exec_error(char *cmd)
{
	struct stat	st;
	char		*copy;

	if (!has_slash(cmd))
		return (err_ret(cmd, "command not found", 127));
	if (stat(cmd, &st) == 0)
	{
		if (S_ISDIR(st.st_mode))
			return (err_ret(cmd, "Is a directory", 126));
		return (err_ret(cmd, "Permission denied", 126));
	}
	copy = ms_strdup(cmd);
	if (copy)
	{
		strip_trailing_slashes(copy);
		if (copy[0] && ms_strcmp(copy, cmd) != 0 && stat(copy, &st) == 0
			&& !S_ISDIR(st.st_mode))
		{
			free(copy);
			return (err_ret(cmd, "Not a directory", 126));
		}
		free(copy);
	}
	return (err_ret(cmd, "No such file or directory", 127));
}
