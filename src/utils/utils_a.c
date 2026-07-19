/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   utils_a.c                                            :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

size_t	ms_strlen(const char *s)
{
	size_t	i;

	i = 0;
	if (!s)
		return (0);
	while (s[i])
		i++;
	return (i);
}

char	*ms_strdup(const char *s)
{
	size_t	len;
	char	*out;
	size_t	i;

	if (!s)
		return (NULL);
	len = ms_strlen(s);
	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	while (i < len)
	{
		out[i] = s[i];
		i++;
	}
	out[len] = '\0';
	return (out);
}

int	ms_strncmp(const char *a, const char *b, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && a[i] && b[i] && a[i] == b[i])
		i++;
	if (i == n)
		return (0);
	return ((unsigned char)a[i] - (unsigned char)b[i]);
}

char	*ms_substr(const char *s, size_t start, size_t len)
{
	char	*out;
	size_t	i;

	if (!s || start > ms_strlen(s))
		return (ms_strdup(""));
	if (len > ms_strlen(s) - start)
		len = ms_strlen(s) - start;
	out = malloc(len + 1);
	if (!out)
		return (NULL);
	i = 0;
	while (i < len)
	{
		out[i] = s[start + i];
		i++;
	}
	out[len] = '\0';
	return (out);
}

void	free_array(char **arr)
{
	int	i;

	if (!arr)
		return ;
	i = 0;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}
