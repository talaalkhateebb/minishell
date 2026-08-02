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
** OLDPWD is one of those on startup, but only when the parent passed no
** value for it: bash inherits a real OLDPWD untouched (so `cd -` works
** immediately), and otherwise still carries the bare name marked for
** export, which is why `export` lists "declare -x OLDPWD" before any cd
** and `cd -` then reports "OLDPWD not set".
*/

static long	get_shlvl_value(const char *cur)
{
	long	lvl;
	int		i;
	int		neg;

	lvl = 0;
	if (!cur || !*cur)
		return (0);
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
	return (lvl);
}

/*
** bash bumps SHLVL by 1 on startup. Negative results clamp to 0; crossing
** 1000 prints a warning and resets to 1.
*/
static void	update_shlvl(t_shell *sh)
{
	long	lvl;
	char	*num;

	lvl = get_shlvl_value(env_get(sh, "SHLVL"));
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

static int	copy_envp(t_shell *sh, char **envp)
{
	int	i;

	i = 0;
	while (envp && envp[i])
	{
		sh->envp[i] = ms_strdup(envp[i]);
		if (!sh->envp[i])
			return (1);
		i++;
	}
	sh->envp[i] = NULL;
	return (0);
}

int	env_init(t_shell *sh, char **envp)
{
	int	n;

	sh->last_status = 0;
	sh->should_exit = 0;
	sh->syntax_token = NULL;
	sh->line = NULL;
	sh->cmds = NULL;
	n = 0;
	while (envp && envp[n])
		n++;
	sh->envp = malloc(sizeof(char *) * (n + 1));
	if (!sh->envp)
		return (1);
	if (copy_envp(sh, envp))
		return (env_free(sh), 1);
	if (env_find_index(sh, "OLDPWD") == -1 && env_set(sh, "OLDPWD", NULL))
		return (env_free(sh), 1);
	return (update_shlvl(sh), 0);
}

void	env_free(t_shell *sh)
{
	free_array(sh->envp);
	sh->envp = NULL;
	free(sh->syntax_token);
	sh->syntax_token = NULL;
}
