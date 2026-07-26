/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_a.c                                         :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Built-ins owned by Person A (env-touching): echo, env.
** export and unset live in builtins_export.c.
*/

/*
** bash echo options: any mix of n/e/E after `-` is a flag word.
** We honour `-n` (no newline); `-e`/`-E` are accepted but ignored.
*/
static int	is_echo_flag(const char *s, int *has_n)
{
	int	i;

	if (!s || s[0] != '-' || !s[1])
		return (0);
	i = 1;
	while (s[i] == 'n' || s[i] == 'e' || s[i] == 'E')
	{
		if (s[i] == 'n')
			*has_n = 1;
		i++;
	}
	return (s[i] == '\0');
}

int	builtin_echo(char **argv)
{
	int	i;
	int	newline;
	int	has_n;

	newline = 1;
	i = 1;
	while (argv[i])
	{
		has_n = 0;
		if (!is_echo_flag(argv[i], &has_n))
			break ;
		if (has_n)
			newline = 0;
		i++;
	}
	while (argv[i])
	{
		put_str(1, argv[i]);
		if (argv[i + 1])
			put_str(1, " ");
		i++;
	}
	if (newline)
		put_str(1, "\n");
	return (0);
}

/*
** `env` prints only entries that actually carry a value — a name that was
** exported without one (`export FOO`) is listed by `export` but not here.
*/
int	builtin_env(t_shell *sh)
{
	int		i;
	char	*eq;

	if (!sh->envp)
		return (0);
	i = 0;
	while (sh->envp[i])
	{
		eq = sh->envp[i];
		while (*eq && *eq != '=')
			eq++;
		if (*eq == '=')
		{
			put_str(1, sh->envp[i]);
			put_str(1, "\n");
		}
		i++;
	}
	return (0);
}
