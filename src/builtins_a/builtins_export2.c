/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_export2.c                                   :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/*
** Bare `export` prints the environment sorted by name, in bash's
** `declare -x KEY="VALUE"` form, with the value quoted. A name exported
** without a value prints as `declare -x KEY` — no `=`, no quotes.
*/

static void	print_export_entry(const char *entry)
{
	size_t	i;

	put_str(1, "declare -x ");
	i = 0;
	while (entry[i] && entry[i] != '=')
		i++;
	write(1, entry, i);
	if (entry[i] != '=')
		return (put_str(1, "\n"));
	put_str(1, "=\"");
	put_str(1, entry + i + 1);
	put_str(1, "\"\n");
}

/* Shallow copy — the pointers are borrowed, so only the array is freed. */
static char	**copy_env_ptrs(t_shell *sh, int *count)
{
	char	**copy;
	int		n;
	int		i;

	n = 0;
	while (sh->envp && sh->envp[n])
		n++;
	*count = n;
	copy = malloc(sizeof(char *) * (n + 1));
	if (!copy)
		return (NULL);
	i = 0;
	while (i < n)
	{
		copy[i] = sh->envp[i];
		i++;
	}
	copy[n] = NULL;
	return (copy);
}

static int	env_name_cmp(const char *a, const char *b)
{
	while (*a && *b && *a != '=' && *b != '=' && *a == *b)
	{
		a++;
		b++;
	}
	if (*a == '=' && *b == '=')
		return (0);
	if (*a == '=')
		return (-1);
	if (*b == '=')
		return (1);
	return ((unsigned char)*a - (unsigned char)*b);
}

static void	sort_ptrs(char **arr, int n)
{
	int		i;
	int		swapped;
	char	*tmp;

	swapped = 1;
	while (swapped)
	{
		swapped = 0;
		i = 0;
		while (i + 1 < n)
		{
			if (env_name_cmp(arr[i], arr[i + 1]) > 0)
			{
				tmp = arr[i];
				arr[i] = arr[i + 1];
				arr[i + 1] = tmp;
				swapped = 1;
			}
			i++;
		}
	}
}

void	print_exports(t_shell *sh)
{
	char	**sorted;
	int		n;
	int		i;

	sorted = copy_env_ptrs(sh, &n);
	if (!sorted)
		return ;
	sort_ptrs(sorted, n);
	i = 0;
	while (i < n)
	{
		print_export_entry(sorted[i]);
		i++;
	}
	free(sorted);
}
