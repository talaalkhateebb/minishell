/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   env_set.c                                            :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*join_kv(const char *key, const char *value)
{
	char	*out;
	size_t	klen;
	size_t	i;

	klen = ms_strlen(key);
	if (!value)
		return (ms_strdup(key));
	out = malloc(klen + ms_strlen(value) + 2);
	if (!out)
		return (NULL);
	i = 0;
	while (i < klen)
	{
		out[i] = key[i];
		i++;
	}
	out[i++] = '=';
	while (*value)
		out[i++] = *value++;
	out[i] = '\0';
	return (out);
}

/* Appends one entry, growing the array by one slot. Takes ownership. */
static int	env_grow(t_shell *sh, char *entry)
{
	char	**bigger;
	int		n;
	int		i;

	n = 0;
	while (sh->envp && sh->envp[n])
		n++;
	bigger = malloc(sizeof(char *) * (n + 2));
	if (!bigger)
		return (free(entry), 1);
	i = 0;
	while (i < n)
	{
		bigger[i] = sh->envp[i];
		i++;
	}
	bigger[n] = entry;
	bigger[n + 1] = NULL;
	free(sh->envp);
	sh->envp = bigger;
	return (0);
}

int	env_set(t_shell *sh, const char *key, const char *value)
{
	char	*entry;
	int		idx;

	if (!key || !*key)
		return (1);
	entry = join_kv(key, value);
	if (!entry)
		return (1);
	idx = env_find_index(sh, key);
	if (idx != -1)
	{
		free(sh->envp[idx]);
		sh->envp[idx] = entry;
		return (0);
	}
	return (env_grow(sh, entry));
}
