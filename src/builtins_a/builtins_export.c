/* ************************************************************************** */
/*                                                                            */
/*                                                          :::      :::::::: */
/*   builtins_export.c                                    :+:      :+:    :+: */
/*                                                        +:+ +:+         +:+ */
/*   By: talaalkh <talaalkh@student.42.fr>                 +#+  +:+       +#+ */
/*                                                          +#+#+#+#+#+   +#+ */
/*   Created: 2026/07/19 11:00:00 by talaalkh                      #+#    #+# */
/*   Updated: 2026/07/19 11:00:00 by talaalkh               ###   ########.fr */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_valid_identifier(const char *s)
{
	int	i;

	if (!s || !is_var_start(s[0]))
		return (0);
	i = 1;
	while (s[i] && s[i] != '=')
	{
		if (!is_var_char(s[i]))
			return (0);
		i++;
	}
	return (1);
}

/*
** `export FOO=bar` sets a value; `export FOO` marks the name exported
** without one, and must NOT clobber an existing FOO.
*/
static int	export_one(char *arg, t_shell *sh)
{
	char	*key;
	int		eq;

	if (!is_valid_identifier(arg))
	{
		put_str(2, "minishell: export: `");
		put_str(2, arg);
		put_str(2, "': not a valid identifier\n");
		return (1);
	}
	eq = 0;
	while (arg[eq] && arg[eq] != '=')
		eq++;
	if (!arg[eq])
	{
		if (env_find_index(sh, arg) == -1)
			return (env_set(sh, arg, NULL));
		return (0);
	}
	key = ms_substr(arg, 0, eq);
	return (env_set(sh, key, arg + eq + 1), free(key), 0);
}

int	builtin_export(char **argv, t_shell *sh)
{
	int	i;
	int	status;

	if (!argv[1])
		return (print_exports(sh), 0);
	status = 0;
	i = 1;
	while (argv[i])
	{
		if (export_one(argv[i], sh))
			status = 1;
		i++;
	}
	return (status);
}

/* `unset FOO=bar` is an error: unset takes bare names, never assignments. */
static int	has_equals(const char *s)
{
	while (*s)
		if (*s++ == '=')
			return (1);
	return (0);
}

int	builtin_unset(char **argv, t_shell *sh)
{
	int	i;
	int	status;

	status = 0;
	i = 1;
	while (argv[i])
	{
		if (!is_valid_identifier(argv[i]) || has_equals(argv[i]))
		{
			put_str(2, "minishell: unset: `");
			put_str(2, argv[i]);
			put_str(2, "': not a valid identifier\n");
			status = 1;
		}
		else
			env_unset(sh, argv[i]);
		i++;
	}
	return (status);
}
