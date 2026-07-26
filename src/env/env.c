/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   env.c                                                :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Env module — Person A (frontend).
**
** Representation: a NULL-terminated char ** of "KEY=VALUE" strings, the
** same shape execve() wants, so env_to_array() is free. An entry with no
** '=' is an exported-but-unset name (`export FOO`): `export` lists it,
** `env` does not, and env_get() reports it as absent.
**
** OLDPWD is one of those on startup: bash drops whatever value the parent
** process had there and only fills it in once this shell has cd'd, so a
** first `cd -` reports "OLDPWD not set" the same way bash's does.
*/

static int	count_envp(char **envp)
{
	int	n;

	n = 0;
	while (envp && envp[n])
		n++;
	return (n);
}

/*
** bash bumps SHLVL by 1 on startup. Negative results clamp to 0; crossing
** 1000 prints a warning and resets to 1.
*/
static void	update_shlvl(t_shell *sh)
{
	char	*cur;
	char	*num;
	long	lvl;
	int		i;
	int		neg;

	cur = env_get(sh, "SHLVL");
	lvl = 0;
	if (cur && *cur)
	{
		i = 0;
		neg = 0;
		if (cur[i] == '-' || cur[i] == '+')
			neg = (cur[i++] == '-');
		while (cur[i] >= '0' && cur[i] <= '9')
			lvl = lvl * 10 + (cur[i++] - '0');
		if (neg)
			lvl = -lvl;
		if (cur[i] != '\0')
			lvl = 0;
	}
	lvl++;
	if (lvl < 0)
		lvl = 0;
	else if (lvl >= 1000)
	{
		num = ms_itoa((int)lvl);
		put_str(2, "minishell: warning: shell level (");
		if (num)
			put_str(2, num);
		put_str(2, ") too high, resetting to 1\n");
		free(num);
		lvl = 1;
	}
	num = ms_itoa((int)lvl);
	if (num)
	{
		env_set(sh, "SHLVL", num);
		free(num);
	}
}

int	env_init(t_shell *sh, char **envp)
{
	int	n;
	int	i;

	sh->last_status = 0;
	n = count_envp(envp);
	sh->envp = malloc(sizeof(char *) * (n + 1));
	if (!sh->envp)
		return (1);
	i = 0;
	while (i < n)
	{
		sh->envp[i] = ms_strdup(envp[i]);
		if (!sh->envp[i])
			return (1);
		i++;
	}
	sh->envp[n] = NULL;
	if (env_set(sh, "OLDPWD", NULL))
		return (1);
	return (update_shlvl(sh), 0);
}

void	env_free(t_shell *sh)
{
	free_array(sh->envp);
	sh->envp = NULL;
}

/*
** True when `entry` ("KEY=VALUE" or bare "KEY") has exactly `key` as its
** name. Comparing up to the '=' stops "PATHEXT=..." matching "PATH".
*/
int	env_key_match(const char *entry, const char *key)
{
	size_t	i;

	i = 0;
	while (entry[i] && entry[i] != '=' && key[i] && entry[i] == key[i])
		i++;
	if (key[i] != '\0')
		return (0);
	return (entry[i] == '=' || entry[i] == '\0');
}

int	env_find_index(t_shell *sh, const char *key)
{
	int	i;

	if (!sh->envp || !key)
		return (-1);
	i = 0;
	while (sh->envp[i])
	{
		if (env_key_match(sh->envp[i], key))
			return (i);
		i++;
	}
	return (-1);
}
