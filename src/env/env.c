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
*/

static int	count_envp(char **envp)
{
	int	n;

	n = 0;
	while (envp && envp[n])
		n++;
	return (n);
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
	return (0);
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
