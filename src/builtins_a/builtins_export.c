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
	int		status;

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
	if (!arg[eq] && env_find_index(sh, arg) != -1)
		return (0);
	if (!arg[eq])
		return (env_set(sh, arg, NULL));
	key = ms_substr(arg, 0, eq);
	if (!key)
		return (1);
	status = env_set(sh, key, arg + eq + 1);
	free(key);
	return (status != 0);
}

int	builtin_export(char **argv, t_shell *sh)
{
	int	i;
	int	status;

	if (!argv[1])
	{
		print_exports(sh);
		return (0);
	}
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

/* `unset FOO=bar` names nothing: unset takes bare names, never assignments. */
static int	has_equals(const char *s)
{
	while (*s)
		if (*s++ == '=')
			return (1);
	return (0);
}

/*
** Unlike export, bash's unset says nothing about a name it cannot use —
** `unset 1BAD`, `unset ""` and `unset a=b` all print no error and leave
** the status at 0. Anything unusable is simply skipped.
*/
int	builtin_unset(char **argv, t_shell *sh)
{
	int	i;

	i = 1;
	while (argv[i])
	{
		if (is_valid_identifier(argv[i]) && !has_equals(argv[i]))
			env_unset(sh, argv[i]);
		i++;
	}
	return (0);
}
