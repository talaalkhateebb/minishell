/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   env_more.c                                           :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** env_get returns a pointer *into* the stored entry, not a copy — callers
** must not free it, and must not hold it across an env_set/env_unset that
** could reallocate the array.
*/
char	*env_get(t_shell *sh, const char *key)
{
	int		idx;
	char	*entry;
	size_t	i;

	idx = env_find_index(sh, key);
	if (idx == -1)
		return (NULL);
	entry = sh->envp[idx];
	i = 0;
	while (entry[i] && entry[i] != '=')
		i++;
	if (entry[i] != '=')
		return (NULL);
	return (entry + i + 1);
}

int	env_unset(t_shell *sh, const char *key)
{
	int	idx;
	int	i;

	idx = env_find_index(sh, key);
	if (idx == -1)
		return (0);
	free(sh->envp[idx]);
	i = idx;
	while (sh->envp[i + 1])
	{
		sh->envp[i] = sh->envp[i + 1];
		i++;
	}
	sh->envp[i] = NULL;
	return (0);
}

char	**env_to_array(t_shell *sh)
{
	return (sh->envp);
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
