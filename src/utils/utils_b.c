/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   utils_b.c                                            :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	ms_strcmp(const char *a, const char *b)
{
	size_t	i;

	i = 0;
	while (a[i] && b[i] && a[i] == b[i])
		i++;
	return ((unsigned char)a[i] - (unsigned char)b[i]);
}

static int	num_len(int n)
{
	int	len;

	len = 1;
	if (n < 0)
		len++;
	while (n / 10 != 0)
	{
		n /= 10;
		len++;
	}
	return (len);
}

char	*ms_itoa(int n)
{
	char			*out;
	int				len;
	unsigned int	nb;

	len = num_len(n);
	out = malloc(len + 1);
	if (!out)
		return (NULL);
	out[len] = '\0';
	if (n < 0)
	{
		out[0] = '-';
		nb = (unsigned int)(-(long)n);
	}
	else
		nb = (unsigned int)n;
	while (len > (n < 0))
	{
		out[--len] = '0' + (nb % 10);
		nb /= 10;
	}
	return (out);
}

int	is_var_start(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_');
}

int	is_var_char(char c)
{
	return (is_var_start(c) || (c >= '0' && c <= '9'));
}
